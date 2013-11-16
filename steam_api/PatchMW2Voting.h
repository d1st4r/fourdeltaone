#pragma once

void Vote_Init();
void Vote_OnInitGame();
void Vote_OnDisconnect(int clientNum);
bool Vote_OnSay(int clientNum, const char* message);
void Vote_OnFrame();