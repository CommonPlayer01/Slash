#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(Location) if(GetWorld()) { DrawDebugSphere(GetWorld(), Location, 50.f, 24, FColor::Green, true, 30.f); }
#define DRAW_SPHERE_COLOR(Location, Color) if(GetWorld()) { DrawDebugSphere(GetWorld(), Location, 8.f, 24, Color, false, 5.f); }
#define DRAW_SPHERE_SingleFrame(Location) if (GetWorld()) { DrawDebugSphere(GetWorld(), Location, 50.f, 24, FColor::Green, false, -1.f); }
#define DRAW_LINE(StartLocation, EndLocation) if(GetWorld()) { DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true, 30.f, 0, 5.f); }
#define DRAW_LINE_SingleFrame(StartLocation, EndLocation) if(GetWorld()) { DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, -1.f, 0, 5.f); }
#define DRAW_POINT(Location) if(GetWorld()) { DrawDebugPoint(GetWorld(), Location, 10.f, FColor::Blue, true, 30.f); }
#define DRAW_POINT_SingleFrame(Location) if(GetWorld()) { DrawDebugPoint(GetWorld(), Location, 10.f, FColor::Blue, false, -1.f); }
#define DRAW_VECTOR(StartLocation, EndLocation) if(GetWorld()) \
	{ \
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true, 30.f, 0, 5.f); \
		DrawDebugPoint(GetWorld(), EndLocation, 10.f, FColor::Blue, true, 30.f);	\
	}
#define DRAW_VECTOR_SingleFrame(StartLocation, EndLocation) if(GetWorld()) \
	{ \
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, -1.f, 0, 5.f); \
		DrawDebugPoint(GetWorld(), EndLocation, 10.f, FColor::Blue, false, -1.f);	\
	}