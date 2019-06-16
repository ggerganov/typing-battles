/*! \file room-words.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include "common.h"

#include <vector>

struct RoomMath : public RoomBase {
    std::function<std::pair<std::string, std::string>(int a, int b)> getQuery;

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

        int a = rand()%100;
        int b = rand()%100;

        auto [ text, answer ] = getQuery(a, b);

        return { x, y, 0, 0.0f, text, answer };
    }

    struct Parameters {
    };

    static std::shared_ptr<RoomBase> getMathAdd(Parameters parameters) {
        auto result = std::make_shared<RoomMath>();

        result->mode = Standard;
        result->name = "Math [+]";
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 1.0f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        result->getQuery = [](int a, int b) -> std::pair<std::string, std::string> {
            return {
                std::to_string(a) + "+" + std::to_string(b),
                std::to_string(a+b),
            };
        };

        return result;
    }

    static std::shared_ptr<RoomBase> getMathSqrt(Parameters parameters) {
        auto result = std::make_shared<RoomMath>();

        result->mode = Standard;
        result->name = "Math [sqrt]";
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 1.0f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        result->getQuery = [](int a, int b) -> std::pair<std::string, std::string> {
            return {
                std::to_string(a*a),
                std::to_string(a),
            };
        };

        return result;
    }

    static std::shared_ptr<RoomBase> getMathAddBR(Parameters parameters) {
        auto result = std::make_shared<RoomMath>();

        result->mode = BattleRoyale;
        result->name = "Math [+,BR]";
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 0.25f*60.f;
        result->tBRRoundLength_s = 0.25f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        result->getQuery = [](int a, int b) -> std::pair<std::string, std::string> {
            return {
                std::to_string(a) + "+" + std::to_string(b),
                std::to_string(a+b),
            };
        };

        return result;
    }

    static std::shared_ptr<RoomBase> getMathSqrtBR(Parameters parameters) {
        auto result = std::make_shared<RoomMath>();

        result->mode = BattleRoyale;
        result->name = "Math [sqrt, BR]";
        result->poolSize = 30;
        result->tCreated_s = timestamp_s();
        result->tRoundLength_s = 0.25f*60.f;
        result->tBRRoundLength_s = 0.25f*60.f;
        result->tRoundStart_s = result->tCreated_s - result->tRoundLength_s;

        result->getQuery = [](int a, int b) -> std::pair<std::string, std::string> {
            return {
                std::to_string(a*a),
                std::to_string(a),
            };
        };

        return result;
    }

};

