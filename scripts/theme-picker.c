#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/fl_draw.H>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Struct to hold wallpaper info
struct Wallpaper {
    std::string subdir;
    Fl_Image *image;
};

// Globals
std::vector<Wallpaper> wallpapers;
int current_index = 0;

// Scroll offset
int scroll_offset = 0;

// Utility: Check if a file exists
bool file_exists(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

// Load a wallpaper file regardless of extension/content
Fl_Image* load_wallpaper(const std::string &path) {
    Fl_Image *img = new Fl_JPEG_Image(path.c_str());
    if (img->w() > 0) return img;
    delete img;

    img = new Fl_PNG_Image(path.c_str());
    if (img->w() > 0) return img;
    delete img;

    return nullptr;
}

// Scan ~/.config/themes
void load_wallpapers() {
    const char *home = getenv("HOME");
    if (!home) return;
    std::string base = std::string(home) + "/.config/themes";

    DIR *dir = opendir(base.c_str());
    if (!dir) { perror("opendir"); return; }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string wallpaper_path = base + "/" + name + "/wallpaper";
        if (!file_exists(wallpaper_path)) continue;

        Fl_Image *img = load_wallpaper(wallpaper_path);
        if (img && img->w() > 0) {
            wallpapers.push_back({name, img});
            std::cout << "Loaded wallpaper: " << name
                      << " " << img->w() << "x" << img->h() << std::endl;
        }
    }

    closedir(dir);
}

// Custom widget for the wallpaper strip
class WallpaperStrip : public Fl_Widget {
public:
    WallpaperStrip(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {
        take_focus();
    }

    Fl_RGB_Image* make_overlay(int w, int h, uchar alpha) {
        uchar* buf = new uchar[w * h * 4]; // RGBA
        for (int i = 0; i < w * h; i++) {
            buf[i*4 + 0] = 0;   // R
            buf[i*4 + 1] = 0;   // G
            buf[i*4 + 2] = 0;   // B
            buf[i*4 + 3] = alpha; // A
        }
        return new Fl_RGB_Image(buf, w, h, 4, 0); // 4 = RGBA
    }
    
    void draw() override {
        // *** Start Double Buffering ***
        Fl_Offscreen offscreen = fl_create_offscreen(w(), h());
        fl_begin_offscreen(offscreen);
        
        // 1. CLEAR THE BACKGROUND (on the buffer)
        // Set background color to black (from the parent window)
        fl_color(parent()->color());
        fl_rectf(0, 0, w(), h()); // Fill the buffer area (coordinates are 0,0 relative to the buffer)

        // 2. SET CLIPPING REGION (relative to buffer: 0, 0, w(), h())
        fl_push_clip(0, 0, w(), h()); 

        // Calculate positioning relative to the widget's origin (0,0)
        int thumb_w = 497;
        int thumb_h = 260;
        int x_pos = 20 - scroll_offset; // Scroll offset affects x_pos directly
        int y_pos = h()/2 - thumb_h/2; 

        static uchar dark_stipple[8] = {
            0xAA, 0x55, 0xAA, 0x55,
            0xAA, 0x55, 0xAA, 0x55
        };

        for (size_t i = 0; i < wallpapers.size(); i++) {
            if (wallpapers[i].image) {
                Fl_Image* thumb = wallpapers[i].image->copy(thumb_w, thumb_h);

                fl_color(parent()->color()); 
                fl_rect(x_pos - 2, y_pos - 2, thumb_w + 4, thumb_h + 4);

                thumb->draw(x_pos, y_pos);

                // Darken unselected images
                if ((int)i != current_index) {
                    static Fl_RGB_Image* overlay = nullptr;
                    if (!overlay || overlay->w() != thumb_w || overlay->h() != thumb_h) {
                        delete overlay;
                        overlay = make_overlay(thumb_w, thumb_h, 120); // 120 ≈ 47% dark
                    }
                    overlay->draw(x_pos, y_pos);
                }

                delete thumb;
            }
            if (i < wallpapers.size() - 1) {
                x_pos += thumb_w + 10;
            } else {
                x_pos += thumb_w + 20;
            }
            // x_pos += thumb_w + 10;
        }

        fl_pop_clip();
        
        // *** End Double Buffering ***
        fl_end_offscreen();
        fl_copy_offscreen(x(), y(), w(), h(), offscreen, 0, 0);
        fl_delete_offscreen(offscreen);
    }

    int handle(int event) override {
        switch(event) {
            case FL_KEYDOWN: {
                int key = Fl::event_key();
                if (key == FL_Left && current_index > 0) {
                    current_index--;
                    scroll_if_needed();
                    redraw();
                    return 1;
                } else if (key == FL_Right && current_index < (int)wallpapers.size()-1) {
                    current_index++;
                    scroll_if_needed();
                    redraw();
                    return 1;
                } else if (key == FL_Enter || key == FL_KP_Enter) {
                    if (current_index >= 0 && current_index < (int)wallpapers.size()) {
                        std::string cmd = std::string("set-theme ") + wallpapers[current_index].subdir;
                        std::cout << "Executing: " << cmd << std::endl;
                        system(cmd.c_str());
                        exit(0);
                    }
                }
                break;
            }
        }
        return Fl_Widget::handle(event);
    }

private:
    void scroll_if_needed() {
        int thumb_w = 497;
        int visible_width = w();
        int left_edge = current_index * (thumb_w + 10);
        int right_edge = left_edge + thumb_w;

        if (left_edge - scroll_offset < 0)
            scroll_offset = left_edge;
        else if (right_edge - scroll_offset > visible_width)
            scroll_offset = right_edge - visible_width;
    }
};

int main(int argc, char **argv) {
    load_wallpapers();

    int screen_w = Fl::w();
    int screen_h = Fl::h();
    int app_w = screen_w - 30;
    int app_h = 300;
    int app_x = 15;
    int app_y = screen_h - app_h - 25;

    Fl_Window *win = new Fl_Window(app_x, app_y, app_w, app_h, "Theme Picker");
    win->color(FL_BLACK);

    WallpaperStrip *strip = new WallpaperStrip(0, 0, app_w, app_h);
    win->set_modal();
    Fl::focus(strip);

    win->end();
    win->show(argc, argv);

    return Fl::run();
}
