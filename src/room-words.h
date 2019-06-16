/*! \file room-words.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include "common.h"

#include <vector>
#include <fstream>

struct RoomWords : public RoomBase {
    using Word = std::string;

    struct Dictionary {
        std::vector<Word> words;

        static Dictionary load(const std::string & path) {
            Dictionary result;

            printf("Loading dictionary from '%s'\n", path.c_str());
            std::ifstream fin(path);
            for (std::string line; getline(fin, line);) {
                result.words.emplace_back(std::move(line));
            }
            fin.close();

            return result;
        }
    };

    Dictionary dictionary;

    virtual Query generate() override {
        float dist = 0.0f;

        float x = 0.0f;
        float y = 0.0f;

        for (int i = 0; i < 32; ++i) {
            float cx = 1.50f*frand() - 0.75f;
            float cy = 1.50f*frand() - 0.75f;

            float curd = 1e6;
            for (auto & query : pool) {
                float dx = cx - query.x;
                float dy = cy - query.y;

                float d = dx*dx + dy*dy;
                if (d < curd) {
                    curd = d;
                }
            }

            if (curd > dist) {
                dist = curd;
                x = cx;
                y = cy;
            }
        }

        int wordId = rand()%dictionary.words.size();

        return { x, y, 0, 0.0f, dictionary.words[wordId], dictionary.words[wordId] };
    }

    struct Parameters {
        std::string pathData = "./";
    };

    static std::shared_ptr<RoomBase> getCpp(Parameters parameters) {
        auto result = std::make_shared<RoomWords>();

        result->mode = Standard;
        result->name = "C++";
        result->dictionary = Dictionary::load(parameters.pathData + "words_cpp.txt");
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 1.0f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        return result;
    }

    static std::shared_ptr<RoomBase> getEnglish(Parameters parameters) {
        auto result = std::make_shared<RoomWords>();

        result->mode = Standard;
        result->name = "English";
        result->dictionary = Dictionary::load(parameters.pathData + "words_alpha.txt");
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 1.0f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        return result;
    }

    static std::shared_ptr<RoomBase> getCppBR(Parameters parameters) {
        auto result = std::make_shared<RoomWords>();

        result->mode = BattleRoyale;
        result->name = "C++ [BR]";
        result->dictionary = Dictionary::load(parameters.pathData + "words_cpp.txt");
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 0.25f*60.f;
        result->tBRRoundLength_s = 0.25f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        return result;
    }

    static std::shared_ptr<RoomBase> getEnglishBR(Parameters parameters) {
        auto result = std::make_shared<RoomWords>();

        result->mode = BattleRoyale;
        result->name = "English [BR]";
        result->dictionary = Dictionary::load(parameters.pathData + "words_alpha.txt");
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 0.25f*60.f;
        result->tBRRoundLength_s = 0.25f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        return result;
    }

};

