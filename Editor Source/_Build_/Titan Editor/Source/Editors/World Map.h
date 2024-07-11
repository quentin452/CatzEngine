#pragma once
/******************************************************************************/
extern bool WorldMapBuilding, WorldMapOk;
extern int WorldMapAreasLeft;
extern State StateWorldMap;
/******************************************************************************/
bool InitWorldMap();
void ShutWorldMap();
bool UpdateWorldMap();
void DrawWorldMap();
/******************************************************************************/
