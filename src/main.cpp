#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include <raylib.h>
#include <entt/entt.hpp>

// ---------------- Components ----------------
struct Position {
    Vector2 position;
};

struct PrevPosition {
    Vector2 prev;
};

struct Velocity {
    Vector2 velocity;
};

struct Sprite {
    Texture texture; // armazenamos o Texture (handle)
};

struct Collider {
    Vector2 size;   // largura/altura da AABB (assumimos origin top-left)
    bool isTrigger; // se true, detecta apenas (sem resolução física)
};

// ---------------- Helpers ----------------
static inline float dot(const Vector2 &a, const Vector2 &b) {
    return a.x * b.x + a.y * b.y;
}

static inline Vector2 reflect(const Vector2 &v, const Vector2 &n) {
    float d = dot(v, n);
    return Vector2{ v.x - 2.0f * d * n.x, v.y - 2.0f * d * n.y };
}

Rectangle MakeAABB(const Position &p, const Collider &c) {
    return Rectangle{ p.position.x, p.position.y, c.size.x, c.size.y };
}

// ---------------- Systems / Algorithms ----------------

// Copia posição atual para PrevPosition (deve ser chamada ANTES de mover)
void CopyPrevPositions(entt::registry &registry) {
    auto view = registry.view<Position, PrevPosition>();
    for (auto entity: view) {
        auto &pos = view.get<Position>(entity);
        auto &pp = view.get<PrevPosition>(entity);
        pp.prev = pos.position;
    }
}

// Move simples (atualiza posição com base na velocidade)
void MoveSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<Position, Velocity>();
    for (auto entity: view) {
        auto &position = view.get<Position>(entity);
        auto &velocity = view.get<Velocity>(entity);
        position.position.x += velocity.velocity.x * deltaTime;
        position.position.y += velocity.velocity.y * deltaTime;
    }
}

// Broad-phase: sweep & prune por eixo X para montar lista de pares candidatos
void BroadPhase(entt::registry &registry,
                std::vector<std::pair<entt::entity, entt::entity>> &outPairs) {
    struct Entry {
        entt::entity ent;
        float minX, maxX;
        float minY, maxY;
    };

    std::vector<Entry> entries;
    auto view = registry.view<Position, Collider>();
    entries.reserve(view.size_hint());

    for (auto e: view) {
        auto &p = view.get<Position>(e);
        auto &c = view.get<Collider>(e);
        Entry en;
        en.ent = e;
        en.minX = p.position.x;
        en.maxX = p.position.x + c.size.x;
        en.minY = p.position.y;
        en.maxY = p.position.y + c.size.y;
        entries.push_back(en);
    }

    // ordenar por minX
    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b){
        return a.minX < b.minX;
    });

    // varredura: para cada entry verificar os seguintes enquanto minX <= maxX do atual
    for (size_t i = 0; i < entries.size(); ++i) {
        for (size_t j = i + 1; j < entries.size(); ++j) {
            if (entries[j].minX > entries[i].maxX) break; // sem mais sobreposição em X
            // checar sobreposição em Y (AABB basic)
            if (entries[i].minY <= entries[j].maxY && entries[i].maxY >= entries[j].minY) {
                outPairs.emplace_back(entries[i].ent, entries[j].ent);
            }
        }
    }
}

// Narrow-phase: Swept AABB (detecção contínua entre duas AABBs)
// Retorna tempo de impacto t no intervalo [0,1] (1.0 = sem colisão nesse frame).
// Se colidiu, normal será preenchido (eixo principal).
float SweptAABB(const Rectangle &A, const Vector2 &vA,
                const Rectangle &B, const Vector2 &vB,
                Vector2 &outNormal, float deltaTime) {
    // movimento relativo do A em relação a B durante o frame
    Vector2 rv = { (vA.x - vB.x) * deltaTime, (vA.y - vB.y) * deltaTime };

    // se já estão sobrepostos no início -> retorno t=0 (colisão imediata)
    if (CheckCollisionRecs(A, B)) {
        outNormal = {0,0};
        return 0.0f;
    }

    // se rv == 0, não se movem relativos -> sem colisão
    if (rv.x == 0.0f && rv.y == 0.0f) return 1.0f;

    float xEntry, xExit, yEntry, yExit;

    if (rv.x > 0.0f) {
        xEntry = (B.x - (A.x + A.width)) / rv.x;
        xExit  = ((B.x + B.width) - A.x) / rv.x;
    } else if (rv.x < 0.0f) {
        xEntry = ((B.x + B.width) - A.x) / rv.x;
        xExit  = (B.x - (A.x + A.width)) / rv.x;
    } else { // sem movimento em x
        xEntry = -INFINITY;
        xExit = INFINITY;
    }

    if (rv.y > 0.0f) {
        yEntry = (B.y - (A.y + A.height)) / rv.y;
        yExit  = ((B.y + B.height) - A.y) / rv.y;
    } else if (rv.y < 0.0f) {
        yEntry = ((B.y + B.height) - A.y) / rv.y;
        yExit  = (B.y - (A.y + A.height)) / rv.y;
    } else {
        yEntry = -INFINITY;
        yExit = INFINITY;
    }

    float entryTime = std::max(xEntry, yEntry);
    float exitTime  = std::min(xExit, yExit);

    // sem colisão durante o intervalo
    if (entryTime > exitTime || entryTime > 1.0f || entryTime < 0.0f) {
        return 1.0f; // sem colisão nesse frame
    }

    // determinamos a normal (qual eixo causou a entrada)
    if (xEntry > yEntry) {
        // colisão ao longo de X
        outNormal.x = (rv.x < 0.0f) ? 1.0f : -1.0f;
        outNormal.y = 0.0f;
    } else {
        outNormal.x = 0.0f;
        outNormal.y = (rv.y < 0.0f) ? 1.0f : -1.0f;
    }

    return entryTime;
}

// Resolução simples: reposiciona no tempo de impacto e reflete velocidade
void NarrowPhaseAndResolve(entt::registry &registry,
                           const std::vector<std::pair<entt::entity, entt::entity>> &pairs,
                           float deltaTime) {
    const float restitution = 0.8f; // coef de restituição simples

    for (auto &pr : pairs) {
        auto eA = pr.first;
        auto eB = pr.second;

        // pegamos componentes; se algum estiver faltando ignore
        if (!registry.all_of<Position, PrevPosition, Velocity, Collider>(eA)) continue;
        if (!registry.all_of<Position, PrevPosition, Velocity, Collider>(eB)) continue;

        auto &posA = registry.get<Position>(eA);
        auto &prevA = registry.get<PrevPosition>(eA);
        auto &velA = registry.get<Velocity>(eA);
        auto &colA = registry.get<Collider>(eA);

        auto &posB = registry.get<Position>(eB);
        auto &prevB = registry.get<PrevPosition>(eB);
        auto &velB = registry.get<Velocity>(eB);
        auto &colB = registry.get<Collider>(eB);

        Rectangle aRect = MakeAABB(posA, colA);
        Rectangle bRect = MakeAABB(posB, colB);

        Vector2 normal;
        float t = SweptAABB(aRect, velA.velocity, bRect, velB.velocity, normal, deltaTime);

        if (t <= 1.0f) {
            // colisão detectada dentro do frame
            // posição no instante do impacto:
            Vector2 impactPosA = { prevA.prev.x + velA.velocity.x * deltaTime * t,
                                   prevA.prev.y + velA.velocity.y * deltaTime * t };
            Vector2 impactPosB = { prevB.prev.x + velB.velocity.x * deltaTime * t,
                                   prevB.prev.y + velB.velocity.y * deltaTime * t };

            // atualizamos posições para instante de impacto
            posA.position = impactPosA;
            posB.position = impactPosB;

            // reflexao simples das velocidades (usando norma)
            Vector2 newVA = reflect(velA.velocity, normal);
            Vector2 newVB = reflect(velB.velocity, normal);

            newVA.x *= restitution;
            newVA.y *= restitution;
            newVB.x *= restitution;
            newVB.y *= restitution;

            // aplicamos movimentos residuais (tempo restante no frame)
            float remaining = (1.0f - t);
            posA.position.x += newVA.x * deltaTime * remaining;
            posA.position.y += newVA.y * deltaTime * remaining;

            posB.position.x += newVB.x * deltaTime * remaining;
            posB.position.y += newVB.y * deltaTime * remaining;

            // atualize as velocidades
            velA.velocity = newVA;
            velB.velocity = newVB;
        } else {
            // sem colisão contínua; porém pode ter interpenetração estática (overlap)
            if (CheckCollisionRecs(aRect, bRect)) {
                // solução simples: separar minimalmente (MTV) e zerar velocidade componente
                float overlapX = std::min(aRect.x + aRect.width, bRect.x + bRect.width) - std::max(aRect.x, bRect.x);
                float overlapY = std::min(aRect.y + aRect.height, bRect.y + bRect.height) - std::max(aRect.y, bRect.y);

                if (overlapX < overlapY) {
                    // separa em X
                    float dir = (aRect.x < bRect.x) ? -1.0f : 1.0f;
                    posA.position.x += dir * (overlapX * 0.5f);
                    posB.position.x -= dir * (overlapX * 0.5f);
                    // zera componente X das velocidades
                    velA.velocity.x = 0.0f;
                    velB.velocity.x = 0.0f;
                } else {
                    // separa em Y
                    float dir = (aRect.y < bRect.y) ? -1.0f : 1.0f;
                    posA.position.y += dir * (overlapY * 0.5f);
                    posB.position.y -= dir * (overlapY * 0.5f);
                    velA.velocity.y = 0.0f;
                    velB.velocity.y = 0.0f;
                }
            }
        }
    }
}

// Render system
void RenderSystem(entt::registry &registry) {
    auto view = registry.view<Position, Sprite>();
    for (auto entity: view) {
        auto &sprite = view.get<Sprite>(entity);
        auto &position = view.get<Position>(entity);
        DrawTexture(sprite.texture, (int)position.position.x, (int)position.position.y, WHITE);
    }

    // opcional: desenhar AABBs para debug
    auto viewC = registry.view<Position, Collider>();
    for (auto e: viewC) {
        auto &p = viewC.get<Position>(e);
        auto &c = viewC.get<Collider>(e);
        DrawRectangleLines((int)p.position.x, (int)p.position.y, (int)c.size.x, (int)c.size.y, GREEN);
    }
}

// ---------------- Main ----------------
int main() {
    InitWindow(800, 600, "raylib + entt - collision demo");
    SetTargetFPS(60);

    Texture2D texture = LoadTexture(RESOURCE_PATH"/sprite.png"); // ajuste o path conforme seu projeto

    entt::registry registry;

    // cria entidades de exemplo
    auto entity1 = registry.create();
    registry.emplace<Position>(entity1, Vector2{10.0f, 10.0f});
    registry.emplace<PrevPosition>(entity1, PrevPosition{ Vector2{10.0f, 10.0f} });
    registry.emplace<Velocity>(entity1, Vector2{200.0f, 25.0f}); // px/s
    registry.emplace<Sprite>(entity1, Sprite{ texture });
    registry.emplace<Collider>(entity1, Collider{ Vector2{(float)texture.width, (float)texture.height}, false });

    auto entity2 = registry.create();
    registry.emplace<Position>(entity2, Vector2{400.0f, 200.0f});
    registry.emplace<PrevPosition>(entity2, PrevPosition{ Vector2{400.0f, 200.0f} });
    registry.emplace<Velocity>(entity2, Vector2{-100.0f, 0.0f});
    registry.emplace<Sprite>(entity2, Sprite{ texture });
    registry.emplace<Collider>(entity2, Collider{ Vector2{(float)texture.width, (float)texture.height}, false });

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        // 1) copie pos anterior
        CopyPrevPositions(registry);

        // 2) movimento simples
        MoveSystem(registry, dt);

        // 3) broad-phase
        std::vector<std::pair<entt::entity, entt::entity>> candidates;
        BroadPhase(registry, candidates);

        // 4) narrow-phase e resolução
        NarrowPhaseAndResolve(registry, candidates, dt);

        // 5) render
        RenderSystem(registry);

        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    return 0;
}
