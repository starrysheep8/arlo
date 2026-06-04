#include <iostream>
#include <fstream>
#include <chrono>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define METER2TIME 5882.352941f

volatile sig_atomic_t connected = false;

const int resolution = 121;
uint8_t vBuffer[resolution][resolution];

int cleanAfterMillis = 3000;
uint64_t lastClean;

void writeVisualBuffer(float worldX, float worldY);
void cleanVisualBuffer();
void clearVisualBuffer();
void drawVisualBuffer();
uint64_t currentTimeMillis();
void sigHandler(int signum);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <PORT>"; 
        return EXIT_FAILURE;
    }

    //Install signal handlers
    if (signal(SIGUSR1, sigHandler) == SIG_ERR) perror("Failed to install SIGUSR1 handler\n");
    if (signal(SIGUSR2, sigHandler) == SIG_ERR) perror("Failed to install SIGUSR2 handler\n");
    if (signal(SIGCHLD, sigHandler) == SIG_ERR) perror("Failed to install SIGCHLD handler\n");

    //Start shell process to read from the port
    // pid_t forkPID = fork();
    // if (forkPID == 0) { //child
    //     char* shellArgs[3] = {"./SaveSerialFile.sh", argv[1], nullptr};
    //     execvp(shellArgs[0], shellArgs);
    // } //parent
    // pause(); //wait for child shell script to signal
    // if (connected == false) {
    //     wait(NULL);
    //     return EXIT_FAILURE;
    // }
    //============= interpret and print the data ============= 

    int portFD = open("/dev/cu.usbmodem1401", O_RDONLY | O_NOCTTY);
    if (portFD < 0) {
        perror("Failed to open port\n");
        return EXIT_FAILURE;
    }

    //throw out first read data in case it's corrupted or half written
    char readChar = '\0';

    while (readChar != '\n') read(portFD, &readChar, 1);

    float x, y;
    char readBuffer[64];
    for (int i = 0; i < 64; i++) readBuffer[i] = '\0';

    lastClean = currentTimeMillis();
    clearVisualBuffer();
    while(true) {
        

        if (currentTimeMillis() - lastClean >= cleanAfterMillis) { //clean buffer after every cleanAfterMillis
            lastClean = currentTimeMillis();
            cleanVisualBuffer();
        }
        for (int i = 0; i < 64; i++) { //read x value
            read(portFD, readBuffer + i, 1);
            if (readBuffer[i] == ' ') {
                readBuffer[i] = '\0';
                x = std::stof(readBuffer);
                break;
            }
        }
        for (int i = 0; i < 64; i++) { //read y value
            read(portFD, readBuffer + i, 1);
            if (readBuffer[i] == '\n') {
                readBuffer[i] = '\0';
                y = std::stof(readBuffer);
                break;
            }
        }
        writeVisualBuffer(x, y);
        drawVisualBuffer(); 
    }
    close(portFD);

    //clean up
    unlink("rawScanData");
    return EXIT_SUCCESS;
}

void writeVisualBuffer(float worldX, float worldY) {
//std::cout << "Writing Visual Buffer: " << worldX << ", " << worldY << " : ";
    if (sqrt(worldX*worldX + worldY*worldY) > 4.1) return;  // don't draw if more than 4 meters away (temporary)

    //convert world coordinates to screen coordinates
    float binScale = (resolution / 2.0f) / 4.1f; 
    vBuffer[(int)(worldX * binScale + resolution / 2.0f)][(int)(worldY * binScale + resolution / 2.0f)] = 4;
//std::cout << (int)(worldX * binScale + resolution / 2.0f) << ", " << (int)(worldY * binScale + resolution / 2.0f) << "\n";
}

void cleanVisualBuffer() {
    for (int y = 0; y < resolution; y++)
        for (int worldX = 0; worldX < resolution; worldX++)
            if (vBuffer[worldX][y] > 0)
                vBuffer[worldX][y]--;
}

void clearVisualBuffer() {for (int y = 0; y < resolution; y++) for (int x = 0; x < resolution; x++) vBuffer[x][y] = 0;}

void drawVisualBuffer() {
    static std::string displayChar[] = {" ", "░", "▒", "▓", "█", "•", "☺︎"};
    vBuffer[resolution/2][resolution/2] = 6;
    for (int y = 0; y < resolution/2+1; y++) {
        for (int x = 0; x < resolution; x++) {
            std::cout << displayChar[vBuffer[x][y]];
        }
        std::cout << '\n';
    }
    for (int i = 0; i < resolution; i++) std::cout << "\033[1A\r";
}

uint64_t currentTimeMillis() { //AI GENERATED
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();

    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        );

    return static_cast<uint64_t>(ms.count());
}

void sigHandler(int signum) {
    if (signum == SIGUSR1) //connected to port
       connected = true; 
    else if (signum == SIGUSR2) //failed to connect to port
        connected = false;
    else if (signum == SIGCHLD) //board disconnected or shell exited
        connected = false;
}