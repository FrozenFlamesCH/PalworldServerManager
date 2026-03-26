#include "Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include "core/Config.h"
#include "core/Database.h"
#include "core/Logger.h"
#include "core/ThreadPool.h"
#include "core/EventBus.h"
#include "net/HttpClient.h"
#include "net/RconClient.h"
#include "net/VanillaAPI.h"
#include "net/PalDefenderAPI.h"
#include "net/PalDefenderRCON.h"
#include "pst/LevelSavParser.h"
#include "server/ServerManager.h"
#include "server/BackupManager.h"
#include "server/ProcessMonitor.h"
#include "scanner/ScanEngine.h"
#include "ui/UIManager.h"
#include "api/RestServer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
}

Application& Application::instance() {
    return *s_instance;
}

bool Application::initialize(int argc, char* argv[]) {
    // Create core subsystems first
    m_logger = std::make_unique<PSM::Logger>();
    m_logger->initialize("palworld_manager.log");
    m_logger->info("Palworld Server Manager v" PSM_VERSION " starting...");

    m_config = std::make_unique<PSM::Config>();
    std::string configPath = "config/settings.json";
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--config") {
            configPath = argv[i + 1];
        }
    }
    if (!m_config->load(configPath)) {
        m_logger->warn("Could not load config from {}, using defaults", configPath);
        m_config->loadDefaults();
    }

    if (!initWindow()) {
        m_logger->error("Failed to initialize window");
        return false;
    }

    if (!initSubsystems()) {
        m_logger->error("Failed to initialize subsystems");
        return false;
    }

    m_running = true;
    m_logger->info("Application initialized successfully");
    return true;
}

bool Application::initWindow() {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    int width = m_config->getInt("window.width", 1600);
    int height = m_config->getInt("window.height", 900);
    bool maximized = m_config->getBool("window.maximized", false);

    if (maximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }

    m_window = glfwCreateWindow(width, height, "Palworld Server Manager", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Load font
    float fontSize = m_config->getFloat("ui.fontSize", 16.0f);
    io.Fonts->AddFontDefault();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

bool Application::initSubsystems() {
    m_threadPool = std::make_unique<PSM::ThreadPool>(
        m_config->getInt("app.threadPoolSize", 4)
    );
    m_eventBus = std::make_unique<PSM::EventBus>();
    m_database = std::make_unique<PSM::Database>();

    std::string dbPath = m_config->getString("database.path", "palworld_manager.db");
    if (!m_database->initialize(dbPath)) {
        m_logger->error("Failed to initialize database");
        return false;
    }

    // Network
    m_httpClient = std::make_unique<PSM::HttpClient>();
    m_rconClient = std::make_unique<PSM::RconClient>();

    m_vanillaAPI = std::make_unique<PSM::VanillaAPI>(
        *m_httpClient,
        m_config->getString("vanilla.host", "127.0.0.1"),
        m_config->getInt("vanilla.port", 8212),
        m_config->getString("vanilla.password", "")
    );

    m_palDefenderAPI = std::make_unique<PSM::PalDefenderAPI>(
        *m_httpClient,
        m_config->getString("paldefender.host", "127.0.0.1"),
        m_config->getInt("paldefender.port", 8214),
        m_config->getString("paldefender.apiKey", "")
    );

    m_palDefenderRCON = std::make_unique<PSM::PalDefenderRCON>(
        *m_rconClient,
        m_config->getString("rcon.host", "127.0.0.1"),
        m_config->getInt("rcon.port", 25575),
        m_config->getString("rcon.password", "")
    );

    // PST
    m_levelSavParser = std::make_unique<PSM::LevelSavParser>(
        m_config->getString("pst.levelSavPath", ""),
        m_config->getInt("pst.syncInterval", 60)
    );

    // Server management
    m_processMonitor = std::make_unique<PSM::ProcessMonitor>(
        m_config->getString("server.exePath", ""),
        m_config->getInt("server.pollInterval", 5)
    );
    m_serverManager = std::make_unique<PSM::ServerManager>(
        *m_vanillaAPI, *m_palDefenderRCON, *m_processMonitor, *m_config, *m_eventBus
    );
    m_backupManager = std::make_unique<PSM::BackupManager>(
        *m_config, *m_logger, *m_database
    );

    // Scanner
    m_scanEngine = std::make_unique<PSM::ScanEngine>(
        *m_levelSavParser, *m_palDefenderAPI, *m_palDefenderRCON,
        *m_database, *m_config, *m_eventBus
    );

    // UI
    m_uiManager = std::make_unique<PSM::UIManager>(*this);

    // REST API server
    if (m_config->getBool("api.enabled", false)) {
        m_restServer = std::make_unique<PSM::RestServer>(
            m_config->getInt("api.port", 8300), *this
        );
    }

    return true;
}

void Application::run() {
    // Start background services
    m_processMonitor->start();
    m_levelSavParser->startAutoSync();
    m_scanEngine->startScheduler();
    m_backupManager->startScheduler();

    if (m_config->getBool("server.autoLaunchOnStart", false)) {
        m_serverManager->startServer();
    }

    if (m_restServer) {
        m_restServer->start();
    }

    while (m_running && !glfwWindowShouldClose(m_window) && !m_shutdownRequested) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_uiManager->render();

        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(m_window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }

        glfwSwapBuffers(m_window);

        // Process events
        m_eventBus->processQueue();
    }
}

void Application::shutdown() {
    m_logger->info("Shutting down...");

    if (m_restServer) m_restServer->stop();
    m_backupManager->stopScheduler();
    m_scanEngine->stopScheduler();
    m_levelSavParser->stopAutoSync();
    m_processMonitor->stop();

    m_config->save();
    m_database->close();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();

    m_logger->info("Shutdown complete");
}

void Application::requestShutdown() {
    m_shutdownRequested = true;
}

PSM::Config& Application::config() { return *m_config; }
PSM::Database& Application::database() { return *m_database; }
PSM::Logger& Application::logger() { return *m_logger; }
PSM::ThreadPool& Application::threadPool() { return *m_threadPool; }
PSM::EventBus& Application::eventBus() { return *m_eventBus; }
PSM::VanillaAPI& Application::vanillaAPI() { return *m_vanillaAPI; }
PSM::PalDefenderAPI& Application::palDefenderAPI() { return *m_palDefenderAPI; }
PSM::PalDefenderRCON& Application::palDefenderRCON() { return *m_palDefenderRCON; }
PSM::LevelSavParser& Application::levelSavParser() { return *m_levelSavParser; }
PSM::ServerManager& Application::serverManager() { return *m_serverManager; }
PSM::BackupManager& Application::backupManager() { return *m_backupManager; }
PSM::ProcessMonitor& Application::processMonitor() { return *m_processMonitor; }
PSM::ScanEngine& Application::scanEngine() { return *m_scanEngine; }
PSM::UIManager& Application::uiManager() { return *m_uiManager; }