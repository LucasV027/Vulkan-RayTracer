#pragma once

#define LAYOUT_STD140 alignas(16)
#define PAD(n) float UNIQUE(_pad, __LINE__)[n]
#define UNIQUE(base, line) UNIQUE_NAME(base, line)
#define UNIQUE_NAME(base, line) base##line
