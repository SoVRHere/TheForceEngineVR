#pragma once

#if defined(ANDROID)
#include "gl_es.h"
#elif defined(_WIN32)
#include "gl_wgl.h"
#endif
