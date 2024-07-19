// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define IM_ASSERT(_EXPR) do { ensure(_EXPR); } while (0)
#define IM_ASSERT_USER_ERROR(_EXP,_MSG) ensureMsgf(_EXP, TEXT(_MSG))

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS
#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef IFileHandle* ImFileHandle;
ImFileHandle ImFileOpen(const char* FileName, const char* Mode);
bool ImFileClose(ImFileHandle File);
uint64 ImFileGetSize(ImFileHandle File);
uint64 ImFileRead(void* Data, uint64 Size, uint64 Count, ImFileHandle File);
uint64 ImFileWrite(const void* Data, uint64 Size, uint64 Count, ImFileHandle File);
#endif

#define ImDrawIdx unsigned int

#define ImTextureID uint32_t

#define IM_VEC2_CLASS_EXTRA \
explicit operator FVector2f() const { return FVector2f(x, y); } \
explicit constexpr ImVec2(const FVector2f& V) : x(V.X), y(V.Y) {} \
explicit operator FVector2d() const { return FVector2d(x, y); } \
explicit constexpr ImVec2(const FVector2d& V) : x(V.X), y(V.Y) {} \
explicit operator FIntPoint() const { return FIntPoint(x, y); } \
explicit constexpr ImVec2(const FIntPoint& V) : x(V.X), y(V.Y) {}

#define IM_VEC4_CLASS_EXTRA \
explicit operator FVector4() const { return FVector4(x, y, z, w); } \
explicit constexpr ImVec4(const FVector4& V) : x(V.X), y(V.Y), z(V.Z), w(V.W) {} \
explicit operator FIntVector4() const { return FIntVector4(x, y, z, w); } \
explicit constexpr ImVec4(const FIntVector4& V) : x(V.X), y(V.Y), z(V.Z), w(V.W) {} \
explicit operator FLinearColor() const { return FLinearColor(x, y, z, w); } \
explicit constexpr ImVec4(const FLinearColor& C) : x(C.R), y(C.G), z(C.B), w(C.A) {}

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
