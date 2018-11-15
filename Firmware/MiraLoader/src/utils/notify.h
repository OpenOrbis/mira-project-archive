#pragma once
#define WriteNotificationLog(fmt) loader_displayNotification(222, fmt)

void loader_displayNotification(int id, char* text);