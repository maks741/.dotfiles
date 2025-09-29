#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/fl_draw.H>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

// Struct to hold wallpaper info
struct Wallpaper {
    std::string subdir;
    Fl_Image *image;
};

// Globals
std::vector<Wallpaper> wallpapers;
int current_index = 0;

// Draw callback for the strip
void draw_strip(Fl_Widget *w, void *) {
    int x = 10;
    int y = w->h()/2 - 50;  // center thumbnails vertically
    int thumb_w = 100;
    int thumb_h = 80;

    for (size_t i = 0; i < wallpapers.size(); i++) {
        if (wallpapers[i].image) {
            wallpapers[i].image->draw(x, y, thumb_w, thumb_h);

            // Highlight current selection
            if ((int)i == current_index) {
                fl_color(FL_RED);
                fl_rect(x-2, y-2, thumb_w+4, thumb_h+4);
            }
        }
        x += thumb_w + 10;
    }
}

// Keyboard handler
int handle_key(int event) {
    if (event == FL_KEYDOWN) {
        int key = Fl::event_key();
        if (key == FL_Left) {
            if (current_index > 0) current_index--;
            Fl::redraw();
            return 1;
        } else if (key == FL_Right) {
            if (current_index < (int)wallpapers.size()-1) current_index++;
            Fl::redraw();
            return 1;
        } else if (key == FL_Enter || key == FL_KP_Enter) {
            if (current_index >= 0 && current_index < (int)wallpapers.size()) {
                std::string cmd = std::string("my_script ") + wallpapers[current_index].subdir;
                std::cout << "Executing: " << cmd << std::endl;
                system(cmd.c_str());
                return 1;
            }
        }
    }
    return 0;
}

// Scan ~/.config/themes
void load_wallpapers() {
    const char *home = getenv("HOME");
    std::string base = std::string(home) + "/.config/themes";

    DIR *dir = opendir(base.c_str());
    if (!dir) { perror("opendir"); return; }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string wallpaper_jpg = base + "/" + name + "/wallpaper.jpg";
        std::string wallpaper_png = base + "/" + name + "/wallpaper.png";

        Fl_Image *img = nullptr;
        struct stat st;

        if (stat(wallpaper_jpg.c_str(), &st) == 0) {
            img = new Fl_JPEG_Image(wallpaper_jpg.c_str());
        } else if (stat(wallpaper_png.c_str(), &st) == 0) {
            img = new Fl_PNG_Image(wallpaper_png.c_str());
        }

        if (img && img->w() > 0) {
            wallpapers.push_back({name, img});
            std::cout << "Loaded wallpaper: " << name << " "
                      << img->w() << "x" << img->h() << std::endl;
        } else if (img) {
            delete img;
        }
    }
    closedir(dir);
}

class WallpaperStrip : public Fl_Widget {
public:
    WallpaperStrip(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}

    void draw() override {
        int x = 10;
        int y = h()/2 - 50;  // center thumbnails vertically
        int thumb_w = 100;
        int thumb_h = 80;

        for (size_t i = 0; i < wallpapers.size(); i++) {
            if (wallpapers[i].image) {
                wallpapers[i].image->draw(x, y, thumb_w, thumb_h);

                // Highlight current selection
                if ((int)i == current_index) {
                    fl_color(FL_RED);
                    fl_rect(x-2, y-2, thumb_w+4, thumb_h+4);
                }
            }
            x += thumb_w + 10;
        }
    }
};

int main(int argc, char **argv) {
    load_wallpapers();

    Fl_Window *win = new Fl_Window(800, 150, "Theme Picker");
    win->color(FL_BLACK);

    WallpaperStrip *strip = new WallpaperStrip(0, 0, 800, 150);

    win->end();
    win->show(argc, argv);

    Fl::add_handler(handle_key);

    return Fl::run();
}

