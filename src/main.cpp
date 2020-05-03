#include <stdio.h>
#include <stdlib.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define MAX_SIZE 32
#define DEFAULT_FREQUENCY 10

typedef struct {
  int demand;
  int frequency;
} Record;

void computeProbabilities(float probabilities[MAX_SIZE],
                          Record records[MAX_SIZE], int numberOfRecords) {
  int sum = 0;
  for (int i = 0; i < numberOfRecords; i++) {
    sum += records[i].frequency;
  }
  for (int i = 0; i < numberOfRecords; i++) {
    probabilities[i] = (float)records[i].frequency / sum;
  }
}

int findRangeIndex(float cumulativeProbabilities[MAX_SIZE], int numberOfRecords,
                   double target) {
  for (int i = 0; i < numberOfRecords - 1; i++) {
    if (target > cumulativeProbabilities[i] &&
        target <= cumulativeProbabilities[i + 1]) {
      return i;
    }
  }
  return numberOfRecords - 1;
}

void computeCumulativeProbabilities(float cumulativeProbabilities[MAX_SIZE],
                                    float probabilities[MAX_SIZE],
                                    int numberOfRecords) {
  float sum = 0.0f;
  for (int i = 0; i < numberOfRecords; i++) {
    sum += probabilities[i];
    cumulativeProbabilities[i] = sum;
  }
}

float getAverageDemand(Record records[MAX_SIZE], float probabilities[MAX_SIZE],
                       int numberOfRecords, int numberOfSamples) {
  float cumulativeProbabilities[MAX_SIZE];
  computeCumulativeProbabilities(cumulativeProbabilities, probabilities,
                                 numberOfRecords);

  srand48(0);
  int total = 0;
  for (int i = 0; i < numberOfSamples; i++) {
    double randomNumber = drand48();
    int rangeIndex =
        findRangeIndex(cumulativeProbabilities, numberOfRecords, randomNumber);
    total += records[rangeIndex].demand;
  }
  return (float)total / numberOfSamples;
}

float getExpectedDemand(Record records[MAX_SIZE], float probabilities[MAX_SIZE],
                        int numberOfRecords) {
  float expectedDemand = 0.0f;
  for (int i = 0; i < numberOfRecords; i++) {
    expectedDemand += records[i].demand * probabilities[i];
  }
  return expectedDemand;
}

int main() {
  glfwInit();
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Inventory Monte Carlo", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glewInit();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 430 core");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static int numberOfRecords = 0;
    static Record records[MAX_SIZE];

    ImGui::Begin("Inventory Monte Carlo");

    ImGui::Columns(2, "data");

    ImGui::Separator();

    ImGui::Text("Demand");
    ImGui::NextColumn();
    ImGui::Text("Frequency");
    ImGui::NextColumn();

    ImGui::Separator();

    for (int i = 0; i < numberOfRecords; i++) {
      Record *record = records + i;
      ImGui::PushID(i);
      if (ImGui::DragInt("##Demand", &record->demand)) {
        record->demand = record->demand > 0 ? record->demand : 0;
      }
      ImGui::NextColumn();
      if (ImGui::DragInt("##Frequency", &record->frequency)) {
        record->frequency = record->frequency > 0 ? record->frequency : 0;
      }
      ImGui::NextColumn();
      ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::Spacing();
    if (ImGui::Button("Add Record")) {
      if (numberOfRecords < MAX_SIZE) {
        Record *record = records + numberOfRecords;
        record->demand = numberOfRecords;
        record->frequency = DEFAULT_FREQUENCY;
        numberOfRecords++;
      }
    }
    ImGui::Spacing();

    ImGui::Separator();

    ImGui::Spacing();
    static int numberOfSamples = 100;
    ImGui::SliderInt("Number Of Samples", &numberOfSamples, 1, 1000000);
    ImGui::Spacing();

    float frequencies[MAX_SIZE];
    computeProbabilities(frequencies, records, numberOfRecords);

    ImGui::Text("Average Demand: %.2f",
                getAverageDemand(records, frequencies, numberOfRecords,
                                 numberOfSamples));

    ImGui::Text("Expected Demand: %.2f",
                getExpectedDemand(records, frequencies, numberOfRecords));

    ImGui::End();

    ImGui::Render();
    int displayWidth, displayHeight;
    glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
    glViewport(0, 0, displayWidth, displayHeight);
    glClearColor(0.05, 0.05, 0.05, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
