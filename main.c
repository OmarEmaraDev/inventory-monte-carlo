#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define NK_PRIVATE
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_IMPLEMENTATION
#define NK_XLIB_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_xlib.h"

#define MAX_SIZE 32
#define DEFAULT_FREQUENCY 10
#define DEFAULT_NUMBER_OF_SAMPLES 5

void getProbabilities(float probabilities[MAX_SIZE], int frequencies[MAX_SIZE],
                      int sampleSize) {
  int total = 0;
  for (int i = 0; i < sampleSize; i++) {
    total += frequencies[i];
  }
  for (int i = 0; i < sampleSize; i++) {
    probabilities[i] = (float)frequencies[i] / total;
  }
}

int findRange(float cumulativeProbabilities[MAX_SIZE], int sampleSize,
              double target) {
  for (int i = 0; i < sampleSize - 1; i++) {
    if (target > cumulativeProbabilities[i] &&
        target <= cumulativeProbabilities[i + 1]) {
      return i;
    }
  }
  return sampleSize - 1;
}

void getCumulativeProbabilities(float cumulativeProbabilities[MAX_SIZE],
                                float probabilities[MAX_SIZE], int sampleSize) {
  float sum = 0.0f;
  for (int i = 0; i < sampleSize; i++) {
    sum += probabilities[i];
    cumulativeProbabilities[i] = sum;
  }
}

float getAverageDemand(float probabilities[MAX_SIZE], int demands[MAX_SIZE],
                       int sampleSize, int numberOfSamples) {
  float cumulativeProbabilities[MAX_SIZE];
  getCumulativeProbabilities(cumulativeProbabilities, probabilities,
                             sampleSize);

  srand48(0);
  int total = 0;
  for (int i = 0; i < numberOfSamples; i++) {
    double randomNumber = drand48();
    total +=
        demands[findRange(cumulativeProbabilities, sampleSize, randomNumber)];
  }
  return (float)total / numberOfSamples;
}

float getExpectedDemand(float probabilities[MAX_SIZE], int demands[MAX_SIZE],
                        int sampleSize) {
  float average = 0.0f;
  for (int i = 0; i < sampleSize; i++) {
    average += demands[i] * probabilities[i];
  }
  return average;
}

int main() {
  Display *display = XOpenDisplay(NULL);
  Window rootWindow = DefaultRootWindow(display);
  int screenNumber = XDefaultScreen(display);
  int screenWidth = XDisplayWidth(display, screenNumber);
  int screenHeight = XDisplayHeight(display, screenNumber);
  Visual *visual = XDefaultVisual(display, screenNumber);
  Colormap colorMap = XCreateColormap(display, rootWindow, visual, AllocNone);

  XSetWindowAttributes setAttributes;
  setAttributes.colormap = colorMap;
  setAttributes.event_mask =
      ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPress |
      ButtonReleaseMask | ButtonMotionMask | Button1MotionMask |
      Button3MotionMask | Button4MotionMask | Button5MotionMask |
      PointerMotionMask | KeymapStateMask;
  Window window =
      XCreateWindow(display, rootWindow, 0, 0, screenWidth, screenHeight, 0,
                    XDefaultDepth(display, screenNumber), InputOutput, visual,
                    CWEventMask | CWColormap, &setAttributes);

  XStoreName(display, window, "Inventory Monte Carlo");
  XMapWindow(display, window);
  Atom wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteWindow, 1);
  XWindowAttributes attributes;
  XGetWindowAttributes(display, window, &attributes);
  unsigned int windowWidth = attributes.width;
  unsigned int windowHeight = attributes.height;

  XFont *font = nk_xfont_create(display, "fixed");
  struct nk_context *context = nk_xlib_init(font, display, screenNumber, window,
                                            windowWidth, windowHeight);

  int sampleSize = 0;
  int demands[MAX_SIZE];
  int frequencies[MAX_SIZE];
  int numberOfSamples = DEFAULT_NUMBER_OF_SAMPLES;

  while (1) {
    XEvent event;
    nk_input_begin(context);
    while (XPending(display)) {
      XNextEvent(display, &event);
      if (event.type == ClientMessage)
        goto cleanup;
      if (XFilterEvent(&event, window))
        continue;
      nk_xlib_handle_event(display, screenNumber, window, &event);
    }
    nk_input_end(context);

    if (nk_begin(context, "Inputs", nk_rect(50, 50, 400, 500),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                     NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
      for (int i = 0; i < sampleSize; i++) {
        nk_layout_row_dynamic(context, 25, 2);
        nk_property_int(context, "#Demand:", 0, demands + i, INT_MAX, 1, 1.0f);
        nk_property_int(context, "#Frequency:", 0, frequencies + i, INT_MAX, 1,
                        1.0f);
      }

      nk_layout_row_static(context, 30, 80, 1);
      if (nk_button_label(context, "Add Sample")) {
        demands[sampleSize] = sampleSize;
        frequencies[sampleSize] = DEFAULT_FREQUENCY;
        sampleSize++;
      }

      nk_layout_row_dynamic(context, 25, 2);
      nk_value_int(context, "Number Of Samples",
                   numberOfSamples * numberOfSamples);
      nk_slider_int(context, 1, &numberOfSamples, 2.0E3, 10);

      float probabilities[MAX_SIZE];
      getProbabilities(probabilities, frequencies, sampleSize);

      nk_layout_row_dynamic(context, 25, 1);
      nk_value_float(context, "Average Demand",
                     getAverageDemand(probabilities, demands, sampleSize,
                                      numberOfSamples * numberOfSamples));
      nk_value_float(context, "Expected Demand",
                     getExpectedDemand(probabilities, demands, sampleSize));
    }
    nk_end(context);

    XClearWindow(display, window);
    nk_xlib_render(window, nk_rgb(30, 30, 30));
    XFlush(display);
  }

cleanup:
  nk_xfont_del(display, font);
  nk_xlib_shutdown();
  XUnmapWindow(display, window);
  XFreeColormap(display, colorMap);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return 0;
}
