#pragma once
#include <string>
#include <vector>
enum cuesh_track_type
{
    CSH_TRK_AUDIO,
    CSH_TRK_OTHER
};
struct cuesh_track
{
    std::string title;
    std::string performer;
    cuesh_track_type type;
    int id;
    int index;
    int hours;
    int minutes;
    int seconds;
};
struct cuesh_data
{
    std::string performer;
    std::string filename;
    std::vector<cuesh_track*> tracks;
};

cuesh_data* cuesh_init(const char* cuefile);
void cuesh_finit(cuesh_data* data);
