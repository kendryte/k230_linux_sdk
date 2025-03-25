#include "pipeline.h"

int main(void) {
    pipeline p = pipeline_create();
    pipeline_run(p);
    return 0;
}