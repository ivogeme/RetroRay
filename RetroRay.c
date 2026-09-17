#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_OBJETOS 10

typedef struct { float x, y, z; } Vetor3D;
typedef struct { float r, g, b; } Cor;

typedef struct {
    Vetor3D centro;
    float raio;
    Cor cor;
    int ativa;
} Esfera;

// Funções matemáticas para o Ray Tracing
Vetor3D sub(Vetor3D a, Vetor3D b) { return (Vetor3D){a.x - b.x, a.y - b.y, a.z - b.z}; }
float dot(Vetor3D a, Vetor3D b)   { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vetor3D norm(Vetor3D v) {
    float tam = sqrtf(dot(v, v));
    return (Vetor3D){v.x / tam, v.y / tam, v.z / tam};
}

// Banco de dados da cena em memória
Esfera esferas[MAX_OBJETOS];
int total_esferas = 0;
Cor cor_fundo = {20, 20, 40}; // Azul escuro clássico de workstation

void adicionar_esfera(float x, float y, float z, float raio, float r, float g, float b) {
    if (total_esferas < MAX_OBJETOS) {
        esferas[total_esferas].centro = (Vetor3D){x, y, z};
        esferas[total_esferas].raio = raio;
        esferas[total_esferas].cor = (Cor){r, g, b};
        esferas[total_esferas].ativa = 1;
        total_esferas++;
        printf("-> OBJETO ADICIONADO: Esfera ID %d em (%.1f, %.1f, %.1f) Raio: %.1f\n", total_esferas-1, x, y, z, raio);
    } else {
        printf("-> ERRO: Limite de objetos atingido!\n");
    }
}

void renderizar_cena() {
    const int LARGURA = 640; // Resolução clássica VGA dos anos 90
    const int ALTURA = 480;
    
    FILE *arquivo = fopen("cena_90s.ppm", "wb");
    if (!arquivo) {
        printf("-> ERRO ao criar arquivo de imagem!\n");
        return;
    }
    fprintf(arquivo, "P6\n%d %d\n255\n", LARGURA, ALTURA);

    Vetor3D camera = {0.0f, 0.0f, 0.0f};
    Vetor3D direcao_luz = norm((Vetor3D){1.0f, 1.0f, 1.0f});

    printf("-> Renderizando %d objeto(s) em 640x480...\n", total_esferas);

    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            // Projeção de perspectiva
            float fx = (float)(x - LARGURA / 2) / (ALTURA / 2);
            float fy = (float)(ALTURA / 2 - y) / (ALTURA / 2);
            Vetor3D dir = norm((Vetor3D){fx, fy, -1.0f});

            unsigned char pixel_r = (unsigned char)cor_fundo.r;
            unsigned char pixel_g = (unsigned char)cor_fundo.g;
            unsigned char pixel_b = (unsigned char)cor_fundo.b;
            
            float t_proximo = 1e20f; // Distância do objeto mais próximo

            // Loop por todos os objetos criados pelo usuário
            for (int i = 0; i < total_esferas; i++) {
                if (!esferas[i].ativa) continue;

                Vetor3D oc = sub(camera, esferas[i].centro);
                float b_val = 2.0f * dot(oc, dir);
                float c_val = dot(oc, oc) - esferas[i].raio * esferas[i].raio;
                float discriminante = b_val * b_val - 4.0f * c_val;

                if (discriminante >= 0.0f) {
                    float t = (-b_val - sqrtf(discriminante)) / 2.0f;
                    if (t > 0.0f && t < t_proximo) {
                        t_proximo = t;
                        Vetor3D hit = {camera.x + t * dir.x, camera.y + t * dir.y, camera.z + t * dir.z};
                        Vetor3D normal = norm(sub(hit, esferas[i].centro));
                        
                        // Iluminação Lambert clássica dos anos 90
                        float iluminacao = dot(normal, direcao_luz);
                        if (iluminacao < 0.0f) iluminacao = 0.0f;
                        iluminacao += 0.2f; // Luz ambiente
                        if (iluminacao > 1.0f) iluminacao = 1.0f;

                        pixel_r = (unsigned char)(esferas[i].cor.r * iluminacao);
                        pixel_g = (unsigned char)(esferas[i].cor.g * iluminacao);
                        pixel_b = (unsigned char)(esferas[i].cor.b * iluminacao);
                    }
                }
            }
            fputc(pixel_r, arquivo);
            fputc(pixel_g, arquivo);
            fputc(pixel_b, arquivo);
        }
    }
    fclose(arquivo);
    printf("-> SUCESSO! Imagem salva no seu computador como 'cena_90s.ppm'\n");
}

int main() {
    char comando[100];
    printf("===========================================\n");
    printf("      SISTEMA DE RENDERIZACAO 3D 1995      \n");
    printf("===========================================\n");
    printf("Comandos disponiveis:\n");
    printf("  esfera [X] [Y] [Z] [Raio] [R] [G] [B]\n");
    printf("  fundo [R] [G] [B]\n");
    printf("  render\n");
    printf("  limpar\n");
    printf("  sair\n\n");

    while (1) {
        printf("3D_Prompt> ");
        if (fgets(comando, sizeof(comando), stdin) == NULL) break;

        if (strncmp(comando, "sair", 4) == 0) {
            break;
        } 
        else if (strncmp(comando, "limpar", 6) == 0) {
            total_esferas = 0;
            printf("-> Cena limpa.\n");
        } 
        else if (strncmp(comando, "render", 6) == 0) {
            renderizar_cena();
        } 
        else if (strncmp(comando, "fundo", 5) == 0) {
            float r, g, b;
            if (sscanf(comando, "fundo %f %f %f", &r, &g, &b) == 3) {
                cor_fundo = (Cor){r, g, b};
                printf("-> Cor de fundo alterada.\n");
            } else {
                printf("-> Erro de sintaxe. Use: fundo R G B\n");
            }
        } 
        else if (strncmp(comando, "esfera", 6) == 0) {
            float x, y, z, raio, r, g, b;
            if (sscanf(comando, "esfera %f %f %f %f %f %f %f", &x, &y, &z, &raio, &r, &g, &b) == 7) {
                adicionar_esfera(x, y, z, raio, r, g, b);
            } else {
                printf("-> Erro de sintaxe. Use: esfera X Y Z Raio R G B\n");
            }
        } 
        else if (strlen(comando) > 1) {
            printf("-> Comando desconhecido.\n");
        }
    }
    return 0;


}