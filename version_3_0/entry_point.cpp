#include <csignal>
#include <string>
#include <cstdlib>

#include "Helper/Logger.hpp"
#include "Configuration/MainConfig.hpp"
#include "Component/EventLoop.hpp"
#include "Component/ServerHttp.hpp"
#include "Component/SignalHandler.hpp"
#include "MemoryPool/ClientPool.hpp"
#include "Types/State.hpp"

extern sig_atomic_t g_signum;

void print_start(void)
{
    Logger::print_info("", "");
    Logger::print_info("******************************", "");
    Logger::print_info("***     START  WEBSERV     ***", "");
    Logger::print_info("******************************", "\n");
}

int print_failed(const std::string &error)
{
    Logger::print_error("[FATAL ERROR]",error.c_str());
    Logger::print_info("******************", "EXIT ERROR\n");
    Logger::print_end();
    return (1);
}

int print_success(void)
{
    Logger::print_info("", "");
    Logger::print_info("******************************", "");
    Logger::print_info("***     EXIT   WEBSERV     ***", "");
    Logger::print_info("******************************", "\n");
    Logger::print_end();
    return (0);
}

int main(const int argc, const char **argv)
{
    {
        srand(time(NULL));
        print_start();
        const char *config_file_path = "config/config.json";
        if (2 == argc) config_file_path = argv[1];
        const MainConfig main_config(config_file_path);
        if (main_config.is_invalid()) return (Logger::print_end(), 1);
        State::init_error_html(main_config);
        EventLoop loop(main_config);
        ClientPool client_pool(main_config.get_max_connections());
        Socket::inject_dependencies(loop, main_config);
        ServerHttp::inject_dependencies(client_pool);
        if (loop.init(&client_pool)) return (Logger::print_end(), 1);
        if (SignalHandler::register_signal_handler(loop)) return (Logger::print_end(), 1);
        Logger::flush();
        try
        {
            loop.run();
        }
        catch(std::exception &e)
        {
            return (print_failed(e.what()));
        }
        SignalHandler::cleanup();
        Logger::print_info("[EXIT   LOOP]", Logger::to_string(g_signum + 128));
    }
    return (print_success());
} // main()