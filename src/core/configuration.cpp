#include "configuration.h"

#include <math.h>
#include <thread>
#include <sys/inotify.h>
#include <unistd.h>
#include <pwd.h>

namespace configuration {
json settings;

void reload(std::string path) {
    std::ifstream file(path);
    try {
        settings = json::parse(file);
        logger::info("Reloaded config file at %s", path.c_str());
    } catch (json::parse_error &e) {
        logger::error("Failed to parse the config file");
    }
    file.close();
}

void init() {
    std::string home_dir = getpwuid(getuid())->pw_dir;

    logger::info("Loading configuration from %s", (home_dir + "/ayypex_settings.json").c_str());

    reload((home_dir + "/ayypex_settings.json").c_str());

    // Start watching for changes
    std::thread inotify_thread([home_dir]() {
        int fd = inotify_init();
        if (fd < 0) {
            logger::error("Failed to initialize inotify");
            return;
        }
        logger::info("Initialized inotify");

        int wd = inotify_add_watch(fd, (home_dir + "/ayypex_settings.json").c_str(), IN_MODIFY);
        if (wd < 0) {
            logger::error("Failed to add inotify watch");
            return;
        }
        logger::info("Added inotify watch");

        char buffer[1024];
        while (true) {
            int length = read(fd, buffer, 1024);
            if (length < 0) {
                logger::error("Failed to read inotify event");
                return;
            }

            int i = 0;
            while (i < length - 1) {
                inotify_event *event = (inotify_event *)&buffer[i];
                if (event->mask & IN_MODIFY) {
                    logger::info("Reloading configuration");
                    reload((home_dir + "/ayypex_settings.json").c_str());
                }
                i += sizeof(inotify_event) + event->len;
            }
        }
    });
    inotify_thread.detach();
    logger::info("Started inotify thread");
}
} // namespace Configuration