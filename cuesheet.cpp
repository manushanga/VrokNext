#include "cuesheet.h"

#include <cstdio>
#include <cstring>
char *read_atom(FILE *fl, char *buffer, int buffer_len, bool *eof) {
    int whitespace_mode = 0;
    int wh = 0;
    int ret = 0;
    buffer[0] = '\0';
    do {
        if (wh >= buffer_len) {
            return nullptr;
        }
        ret = fread(&buffer[wh], 1, 1, fl);
        if (ret == 0) {
            *eof = true;
            return buffer;
        }

        if (buffer[wh] == '\n') {
            buffer[wh] = '\0';
            return buffer;
        }

        if (whitespace_mode == 0) {
            if ((buffer[wh] != ' ') && (buffer[wh] != '\t') && (buffer[wh] != '\r')) {
                if (buffer[wh] == '"') {
                    whitespace_mode = 2;
                } else {
                    whitespace_mode = 1;
                    wh++;
                }
            }
        } else if (whitespace_mode == 1) {
            if (buffer[wh] == ' ') {
                buffer[wh] = '\0';
                return buffer;
            } else if (buffer[wh] == '"') {
                whitespace_mode = 2;
            }
            wh++;
        } else if (whitespace_mode == 2) {
            if (buffer[wh] == '"') {
                whitespace_mode = 1;
            } else {
                wh++;
            }
        }
    } while (ret != 0);
    return buffer;
}
void read_title(FILE *fl, bool *iseof, cuesh_track *track) {
    char atombuf[256];
    char *atomtitl = read_atom(fl, atombuf, 256, iseof);
    if (!*iseof && atomtitl) {
        track->title = std::string(atomtitl);
    }
}
void read_performer(FILE *fl, bool *iseof, cuesh_track *track) {
    char atombuf[256];
    char *atomperf = read_atom(fl, atombuf, 256, iseof);
    if (!iseof && atomperf) {
        track->performer = std::string(atomperf);
    }
}

void read_index(FILE *fl, bool *iseof, cuesh_track *track) {
    char atombuf[256];
    char *atomindex = read_atom(fl, atombuf, 256, iseof);
    if (!*iseof && atomindex) {
        track->index = atoi(atomindex);
        *iseof = false;
        char *duration = read_atom(fl, atombuf, 256, iseof);
        if (!*iseof && duration) {
            int frames;
            track->hours = 0;
            sscanf(duration, "%02d:%02d:%02d", &track->minutes, &track->seconds, &frames);
        }
    }
}
void read_track(FILE *fl, bool *iseof, cuesh_track *track) {
    char atombuf[256];
    char *atomid = read_atom(fl, atombuf, 256, iseof);
    if (!*iseof && atomid) {
        track->id = atoi(atomid);
    }
    char *atomkind = read_atom(fl, atombuf, 256, iseof);
    if (!*iseof && atomkind) {
        track->type = CSH_TRK_OTHER;
        if (strcmp(atomkind, "AUDIO") == 0) {
            track->type = CSH_TRK_AUDIO;
        }
    }

    return;
}
cuesh_data *cuesh_init(const char *cuefile) {
    FILE *fl = fopen(cuefile, "r");
    cuesh_data *cd = new cuesh_data();
    cuesh_track *current_track = nullptr;
    while (!feof(fl)) {
        char atombuf[256];
        bool iseof = false;
        char *atom = read_atom(fl, atombuf, 32, &iseof);
        if (!iseof) {
            iseof = false;
            if (atom == nullptr) {
                return nullptr;
            }
            if (strcmp(atom, "REM") == 0) {
                printf("%s\n", atom);
            } else if (strcmp(atom, "PERFORMER") == 0) {
                char *atomperf = read_atom(fl, atombuf, 256, &iseof);
                if (!iseof && atomperf) {
                    if (current_track) {
                        current_track->performer = std::string(atomperf);
                    } else {
                        cd->performer = std::string(atomperf);
                    }
                }
            } else if (strcmp(atom, "TRACK") == 0) {
                if (current_track) {
                    cd->tracks.push_back(current_track);
                    current_track = nullptr;
                }
                current_track = new cuesh_track();
                read_track(fl, &iseof, current_track);
            } else if (strcmp(atom, "TITLE") == 0) {
                if (current_track)
                    read_title(fl, &iseof, current_track);
            } else if (strcmp(atom, "INDEX") == 0) {
                if (current_track)
                    read_index(fl, &iseof, current_track);
            } else if (strcmp(atom, "FILE") == 0) {
                char *atomfile = read_atom(fl, atombuf, 256, &iseof);
                if (!iseof && atomfile) {
                    cd->filename = std::string(atomfile);
                }
            }
        }
    }
    printf("%s\n", cd->filename.c_str());
    return cd;
}

void cuesh_finit(cuesh_data *data) {
    for (auto &d : data->tracks) {
        delete d;
    }
}
