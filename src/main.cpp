#include <stdio.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <random>

#define MAX_SIZE 32
#define DEFAULT_FREQUENCY 10
#define DEFAULT_NUMBER_OF_SIMULATION_DAYS 10
#define DEFAULT_ORDER_QUANTITY 10
#define DEFAULT_REORDER_POINT 5
#define START_INVENTORY 5

struct Record {
  int value;
  int frequency;
};

typedef std::default_random_engine RandomEngine;
typedef std::uniform_real_distribution<double> RandomDistribution;

void computeCumulativeProbabilities(Record records[MAX_SIZE],
                                    int numberOfRecords,
                                    double cumulativeProbabilities[MAX_SIZE]) {
  int total = 0;
  for (int i = 0; i < numberOfRecords; i++) {
    total += records[i].frequency;
  }
  double sum = 0.0f;
  for (int i = 0; i < numberOfRecords; i++) {
    sum += (double)records[i].frequency / total;
    cumulativeProbabilities[i] = sum;
  }
}

int findRangeIndex(double array[MAX_SIZE], int arraySize, double target) {
  for (int i = 0; i < arraySize - 1; i++) {
    if (target > array[i] && target <= array[i + 1]) {
      return i;
    }
  }
  return arraySize - 1;
}

int getRandomRecordValue(double cumulativeProbabilities[MAX_SIZE],
                         Record records[MAX_SIZE], int numberOfRecords,
                         RandomEngine &randomEngine,
                         RandomDistribution &randomDistribution) {
  int rangeIndex = findRangeIndex(cumulativeProbabilities, numberOfRecords,
                                  randomDistribution(randomEngine));
  return records[rangeIndex].value;
}

void analyseInventory(Record demandRecords[MAX_SIZE], int numberOfDemandRecords,
                      Record leadTimeRecords[MAX_SIZE],
                      int numberOfLeadTimeRecords, int numberOfSimulationDays,
                      int orderQuantity, int reorderPoint,
                      RandomEngine &randomEngine,
                      RandomDistribution &randomDistribution,
                      double *averageEndingInventory, double *averageLostSales,
                      double *averageNumberOfOrders) {
  double demandCumulativeProbabilities[MAX_SIZE];
  computeCumulativeProbabilities(demandRecords, numberOfDemandRecords,
                                 demandCumulativeProbabilities);

  double leadTimeCumulativeProbabilities[MAX_SIZE];
  computeCumulativeProbabilities(leadTimeRecords, numberOfLeadTimeRecords,
                                 leadTimeCumulativeProbabilities);

  int numberOfOrdersPlaced = 0;
  int sumOfEndingInventory = 0;
  int sumOfLostSales = 0;
  int currentInventory = START_INVENTORY;
  int daysUntilArrivalOfOrder = 0;
  bool isWaitingForOrder = false;
  for (int i = 0; i < numberOfSimulationDays; i++) {
    if (isWaitingForOrder && daysUntilArrivalOfOrder == 0) {
      currentInventory += orderQuantity;
      isWaitingForOrder = false;
    }
    daysUntilArrivalOfOrder--;

    int demand = getRandomRecordValue(demandCumulativeProbabilities,
                                      demandRecords, numberOfDemandRecords,
                                      randomEngine, randomDistribution);
    currentInventory -= demand;
    if (currentInventory < 0) {
      sumOfLostSales -= currentInventory;
      currentInventory = 0;
    } else {
      sumOfEndingInventory += currentInventory;
    }

    if (currentInventory < reorderPoint) {
      daysUntilArrivalOfOrder = getRandomRecordValue(
          leadTimeCumulativeProbabilities, leadTimeRecords,
          numberOfLeadTimeRecords, randomEngine, randomDistribution);
      isWaitingForOrder = true;
      numberOfOrdersPlaced++;
    }
  }

  *averageEndingInventory =
      (double)sumOfEndingInventory / numberOfSimulationDays;
  *averageLostSales = (double)sumOfLostSales / numberOfSimulationDays;
  *averageNumberOfOrders =
      (double)numberOfOrdersPlaced / numberOfSimulationDays;
}

int main() {
  glfwInit();
  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Inventory Monte Carlo", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  gladLoadGL();

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

    static int numberOfDemandRecords = 0;
    static Record demandRecords[MAX_SIZE];
    static int numberOfLeadTimeRecords = 0;
    static Record leadTimeRecords[MAX_SIZE];

    ImGui::Begin("Inventory Monte Carlo");

    ImGui::Columns(2, "Demands Table");

    ImGui::Separator();

    ImGui::Text("Demand");
    ImGui::NextColumn();
    ImGui::Text("Frequency");
    ImGui::NextColumn();

    ImGui::Separator();

    for (int i = 0; i < numberOfDemandRecords; i++) {
      Record *record = demandRecords + i;
      ImGui::PushID(i);
      if (ImGui::DragInt("##Demand", &record->value)) {
        record->value = record->value > 0 ? record->value : 0;
      }
      ImGui::NextColumn();
      if (ImGui::DragInt("##Demand Frequency", &record->frequency)) {
        record->frequency = record->frequency > 0 ? record->frequency : 0;
      }
      ImGui::NextColumn();
      ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::Spacing();
    if (ImGui::Button("Add Demand Record")) {
      if (numberOfDemandRecords < MAX_SIZE) {
        Record *record = demandRecords + numberOfDemandRecords;
        record->value = numberOfDemandRecords;
        record->frequency = DEFAULT_FREQUENCY;
        numberOfDemandRecords++;
      }
    }
    ImGui::Spacing();

    ImGui::Columns(2, "Lead Times Table");

    ImGui::Separator();

    ImGui::Text("Lead Time");
    ImGui::NextColumn();
    ImGui::Text("Frequency");
    ImGui::NextColumn();

    ImGui::Separator();

    for (int i = 0; i < numberOfLeadTimeRecords; i++) {
      Record *record = leadTimeRecords + i;
      ImGui::PushID(i);
      if (ImGui::DragInt("##Lead Time", &record->value)) {
        record->value = record->value > 0 ? record->value : 0;
      }
      ImGui::NextColumn();
      if (ImGui::DragInt("##Lead Time Frequency", &record->frequency)) {
        record->frequency = record->frequency > 0 ? record->frequency : 0;
      }
      ImGui::NextColumn();
      ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::Spacing();
    if (ImGui::Button("Add Lead Time Record")) {
      if (numberOfLeadTimeRecords < MAX_SIZE) {
        Record *record = leadTimeRecords + numberOfLeadTimeRecords;
        record->value = numberOfLeadTimeRecords;
        record->frequency = DEFAULT_FREQUENCY;
        numberOfLeadTimeRecords++;
      }
    }
    ImGui::Spacing();

    ImGui::Separator();

    ImGui::Spacing();
    static int numberOfSimulationDays = DEFAULT_NUMBER_OF_SIMULATION_DAYS;
    ImGui::SliderInt("Number Of Simulation Days", &numberOfSimulationDays, 1,
                     1000);
    static int orderQuantity = DEFAULT_ORDER_QUANTITY;
    ImGui::SliderInt("Order Quantity", &orderQuantity, 1, 50);
    static int reorderPoint = DEFAULT_REORDER_POINT;
    ImGui::SliderInt("Rorder Point", &reorderPoint, 1, 10);
    ImGui::Spacing();

    RandomEngine randomEngine;
    RandomDistribution randomDistribution(0.0, 1.0);

    double averageEndingInventory, averageLostSales, averageNumberOfOrders;
    analyseInventory(demandRecords, numberOfDemandRecords, leadTimeRecords,
                     numberOfLeadTimeRecords, numberOfSimulationDays,
                     orderQuantity, reorderPoint, randomEngine,
                     randomDistribution, &averageEndingInventory,
                     &averageLostSales, &averageNumberOfOrders);

    ImGui::Text("Average Ending Inventory: %.2f", averageEndingInventory);
    ImGui::Text("Average Lost Sales: %.2f", averageLostSales);
    ImGui::Text("Average Number Of Orders: %.2f", averageNumberOfOrders);

    ImGui::End();

    ImGui::Render();
    int displayWidth, displayHeight;
    glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
    glViewport(0, 0, displayWidth, displayHeight);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
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
