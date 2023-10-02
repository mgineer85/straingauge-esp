#pragma once

#include <Arduino.h>
#include <FFat.h>

///

void printFile(const String filename);
void printDirectory(File dir, int numTabs = 1);