#ifndef CHATGPT_WRAPPER_SETTINGS_H
#define CHATGPT_WRAPPER_SETTINGS_H

typedef struct {
    int width;
    int height;
} WindowSettings;

WindowSettings settings_load_window(void);
void settings_store_window(int width, int height);

#endif
