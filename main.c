#include "raylib.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PRODUITS 100
#define MAX_VENTES 1000
#define MAX_INPUT_LENGTH 50

typedef struct {
    char nom[50];
    float prix;
    int stock;
    int id;
} Produit;

typedef struct {
    Produit produit;
    int quantite;
    time_t date;
    float prixTotal;
} Vente;

typedef struct {
    Produit produits[MAX_PRODUITS];
    Vente ventes[MAX_VENTES];
    int nombreProduits;
    int nombreVentes;
    int ecranActuel; // 0=menu, 1=produits, 2=ventes, 3=stats, 4=historique
    bool showAjoutProduit;
    bool showEditProduit;
    bool showDeleteConfirm;
    bool showNotification;
    char messageNotification[100];
    int timerNotification;
    char nouveauNom[50];
    char nouveauPrix[20];
    char nouveauStock[20];
    int produitSelectionne;
    int saisieActive; // 0=rien, 1=nom, 2=prix, 3=stock, 4=quantite, 5=recherche
    int ventePage;
    int statsVue; // 0=jour, 1=semaine, 2=mois
    char rechercheTexte[50];
    bool showRechercheBar;
    bool trierParNom;
    bool trierParPrix;
    bool trierParStock;
    bool trierAscendant;
} GestionCommerciale;

// Prototypes
void InitialiserDonnees(GestionCommerciale *gestion);
void SauvegarderDonnees(GestionCommerciale *gestion);
void ChargerDonnees(GestionCommerciale *gestion);
void DessinerMenu(GestionCommerciale *gestion);
void DessinerEcranProduits(GestionCommerciale *gestion);
void DessinerEcranVentes(GestionCommerciale *gestion);
void DessinerEcranHistoriqueVentes(GestionCommerciale *gestion);
void DessinerEcranStats(GestionCommerciale *gestion);
void DessinerFormulaireProduit(GestionCommerciale *gestion, bool estEdition);
void DessinerConfirmationSuppression(GestionCommerciale *gestion);
void AjouterProduit(GestionCommerciale *gestion);
void ModifierProduit(GestionCommerciale *gestion);
void SupprimerProduit(GestionCommerciale *gestion, int index);
void EnregistrerVente(GestionCommerciale *gestion, int idProduit, int quantite);
bool Bouton(int x, int y, int largeur, int hauteur, const char *texte, Color couleur, Color couleurHover);
bool BoutonIcone(int x, int y, int taille, const char *icone, Color couleur);
void GererSaisieTexte(char *texte, int maxLength, bool *active, bool numerique);
void AfficherNotification(GestionCommerciale *gestion, const char *message);
void TrierProduits(GestionCommerciale *gestion);
void DessinerBarreRecherche(GestionCommerciale *gestion);

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Gestion Commerciale Pro");
    SetTargetFPS(60);
    
    GestionCommerciale gestion = {0};
    InitialiserDonnees(&gestion);
    ChargerDonnees(&gestion);
    
    int frameCounter = 0;
    
    while (!WindowShouldClose()) {
        frameCounter++;
        
        // Gestion des entrées
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (gestion.showAjoutProduit || gestion.showEditProduit || gestion.showDeleteConfirm) {
                gestion.showAjoutProduit = false;
                gestion.showEditProduit = false;
                gestion.showDeleteConfirm = false;
            } else {
                gestion.ecranActuel = 0;
            }
        }
        
        // Gestion du timer de notification
        if (gestion.showNotification) {
            gestion.timerNotification--;
            if (gestion.timerNotification <= 0) {
                gestion.showNotification = false;
            }
        }
        
        // Dessin
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // En-tête commune
        DrawRectangle(0, 0, screenWidth, 80, DARKBLUE);
        DrawRectangle(0, 80, screenWidth, 2, BLUE);
        DrawText("GESTION COMMERCIALE PRO", 40, 25, 30, WHITE);
        
        // Boutons de navigation
        if (gestion.ecranActuel != 0) {
            if (Bouton(screenWidth - 140, 20, 120, 40, "MENU", MAROON, RED)) {
                gestion.ecranActuel = 0;
            }
        }
        
        // Écran actuel
        switch(gestion.ecranActuel) {
            case 0: DessinerMenu(&gestion); break;
            case 1: DessinerEcranProduits(&gestion); break;
            case 2: DessinerEcranVentes(&gestion); break;
            case 3: DessinerEcranStats(&gestion); break;
            case 4: DessinerEcranHistoriqueVentes(&gestion); break;
        }
        
        // Formulaires modaux
        if (gestion.showAjoutProduit) {
            DessinerFormulaireProduit(&gestion, false);
        }
        
        if (gestion.showEditProduit) {
            DessinerFormulaireProduit(&gestion, true);
        }
        
        if (gestion.showDeleteConfirm) {
            DessinerConfirmationSuppression(&gestion);
        }
        
        // Notifications
        if (gestion.showNotification) {
            int notifWidth = MeasureText(gestion.messageNotification, 20) + 40;
            DrawRectangleRounded((Rectangle){screenWidth/2 - notifWidth/2, 90, notifWidth, 40}, 0.3f, 10, Fade(DARKBLUE, 0.9f));
            DrawText(gestion.messageNotification, screenWidth/2 - MeasureText(gestion.messageNotification, 20)/2, 100, 20, WHITE);
        }
        
        EndDrawing();
    }
    
    SauvegarderDonnees(&gestion);
    CloseWindow();
    return 0;
}

void InitialiserDonnees(GestionCommerciale *gestion) {
    // Produits par défaut
    strcpy(gestion->produits[0].nom, "T-Shirt");
    gestion->produits[0].prix = 19.99f;
    gestion->produits[0].stock = 50;
    gestion->produits[0].id = 1;
    
    strcpy(gestion->produits[1].nom, "Jean");
    gestion->produits[1].prix = 49.99f;
    gestion->produits[1].stock = 30;
    gestion->produits[1].id = 2;
    
    strcpy(gestion->produits[2].nom, "Chaussures");
    gestion->produits[2].prix = 79.99f;
    gestion->produits[2].stock = 20;
    gestion->produits[2].id = 3;
    
    gestion->nombreProduits = 3;
    gestion->ecranActuel = 0;
    gestion->ventePage = 0;
    gestion->statsVue = 1;
    gestion->trierParNom = true;
    gestion->trierAscendant = true;
}

void SauvegarderDonnees(GestionCommerciale *gestion) {
    FILE *fichierProduits = fopen("produits.dat", "wb");
    if (fichierProduits) {
        fwrite(&gestion->nombreProduits, sizeof(int), 1, fichierProduits);
        fwrite(gestion->produits, sizeof(Produit), gestion->nombreProduits, fichierProduits);
        fclose(fichierProduits);
    }
    
    FILE *fichierVentes = fopen("ventes.dat", "wb");
    if (fichierVentes) {
        fwrite(&gestion->nombreVentes, sizeof(int), 1, fichierVentes);
        fwrite(gestion->ventes, sizeof(Vente), gestion->nombreVentes, fichierVentes);
        fclose(fichierVentes);
    }
}

void ChargerDonnees(GestionCommerciale *gestion) {
    FILE *fichierProduits = fopen("produits.dat", "rb");
    if (fichierProduits) {
        fread(&gestion->nombreProduits, sizeof(int), 1, fichierProduits);
        fread(gestion->produits, sizeof(Produit), gestion->nombreProduits, fichierProduits);
        fclose(fichierProduits);
    }
    
    FILE *fichierVentes = fopen("ventes.dat", "rb");
    if (fichierVentes) {
        fread(&gestion->nombreVentes, sizeof(int), 1, fichierVentes);
        fread(gestion->ventes, sizeof(Vente), gestion->nombreVentes, fichierVentes);
        fclose(fichierVentes);
    }
}

// Fix for the Bouton function
bool Bouton(int x, int y, int largeur, int hauteur, const char *texte, Color couleur, Color couleurHover) {
    Rectangle btnBounds = {x, y, largeur, hauteur};
    bool hover = CheckCollisionPointRec(GetMousePosition(), btnBounds);
    bool click = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    DrawRectangleRounded(btnBounds, 0.2f, 8, hover ? couleurHover : couleur);
    // Fix: DrawRectangleRoundedLines has only 4 parameters, not 5
    DrawRectangleRoundedLines(btnBounds, 0.2f, 8, ColorAlpha(BLACK, 0.3f));
    
    int textWidth = MeasureText(texte, 20);
    DrawText(texte, x + largeur/2 - textWidth/2, y + hauteur/2 - 10, 20, WHITE);
    
    return click;
}


bool BoutonIcone(int x, int y, int taille, const char *icone, Color couleur) {
    Rectangle btnBounds = {x, y, taille, taille};
    bool hover = CheckCollisionPointRec(GetMousePosition(), btnBounds);
    bool click = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    DrawRectangleRounded(btnBounds, 0.3f, 8, hover ? ColorAlpha(couleur, 0.8f) : couleur);
    
    int textWidth = MeasureText(icone, 20);
    DrawText(icone, x + taille/2 - textWidth/2, y + taille/2 - 10, 20, WHITE);
    
    return click;
}

void GererSaisieTexte(char *texte, int maxLength, bool *active, bool numerique) {
    if (*active) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(texte) < maxLength - 1)) {
                // Pour les champs numériques, n'accepter que les chiffres et le point
                if (!numerique || (key >= '0' && key <= '9') || key == '.') {
                    strncat(texte, (char *)&key, 1);
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE) && strlen(texte) > 0) {
            texte[strlen(texte)-1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_TAB)) {
            *active = false;
        }
    }
}

void AfficherNotification(GestionCommerciale *gestion, const char *message) {
    gestion->showNotification = true;
    strcpy(gestion->messageNotification, message);
    gestion->timerNotification = 180; // 3 secondes à 60 FPS
}

void DessinerMenu(GestionCommerciale *gestion) {
    int centerX = GetScreenWidth() / 2;
    int btnWidth = 300;
    int btnHeight = 60;
    int btnSpace = 20;
    int startY = 200;
    
    DrawText("MENU PRINCIPAL", centerX - MeasureText("MENU PRINCIPAL", 40)/2, 120, 40, DARKBLUE);
    
    // Dessiner les boutons principaux
    if (Bouton(centerX - btnWidth/2, startY, btnWidth, btnHeight, "GESTION DES PRODUITS", DARKBLUE, BLUE)) {
        gestion->ecranActuel = 1;
    }
    
    if (Bouton(centerX - btnWidth/2, startY + btnHeight + btnSpace, btnWidth, btnHeight, "ENREGISTRER VENTE", DARKBLUE, BLUE)) {
        gestion->ecranActuel = 2;
    }
    
    if (Bouton(centerX - btnWidth/2, startY + 2*(btnHeight + btnSpace), btnWidth, btnHeight, "HISTORIQUE DES VENTES", DARKBLUE, BLUE)) {
        gestion->ecranActuel = 4;
    }
    
    if (Bouton(centerX - btnWidth/2, startY + 3*(btnHeight + btnSpace), btnWidth, btnHeight, "STATISTIQUES", DARKBLUE, BLUE)) {
        gestion->ecranActuel = 3;
    }
    
    // Version
    DrawText("v2.0", GetScreenWidth() - 50, GetScreenHeight() - 30, 20, GRAY);
}

void TrierProduits(GestionCommerciale *gestion) {
    // Tri à bulles simple
    for (int i = 0; i < gestion->nombreProduits - 1; i++) {
        for (int j = 0; j < gestion->nombreProduits - i - 1; j++) {
            bool echange = false;
            
            if (gestion->trierParNom) {
                if (gestion->trierAscendant) {
                    echange = strcmp(gestion->produits[j].nom, gestion->produits[j+1].nom) > 0;
                } else {
                    echange = strcmp(gestion->produits[j].nom, gestion->produits[j+1].nom) < 0;
                }
            } else if (gestion->trierParPrix) {
                if (gestion->trierAscendant) {
                    echange = gestion->produits[j].prix > gestion->produits[j+1].prix;
                } else {
                    echange = gestion->produits[j].prix < gestion->produits[j+1].prix;
                }
            } else if (gestion->trierParStock) {
                if (gestion->trierAscendant) {
                    echange = gestion->produits[j].stock > gestion->produits[j+1].stock;
                } else {
                    echange = gestion->produits[j].stock < gestion->produits[j+1].stock;
                }
            }
            
            if (echange) {
                Produit temp = gestion->produits[j];
                gestion->produits[j] = gestion->produits[j+1];
                gestion->produits[j+1] = temp;
            }
        }
    }
}

void DessinerBarreRecherche(GestionCommerciale *gestion) {
    int x = 40;
    int y = 120;
    
    DrawText("Rechercher:", x, y, 20, DARKBLUE);
    
    Rectangle rechercheRect = {x + 150, y, 300, 30};
    bool rechercheActive = (gestion->saisieActive == 5);
    
    DrawRectangleRec(rechercheRect, rechercheActive ? YELLOW : WHITE);
    DrawRectangleLinesEx(rechercheRect, 1, DARKGRAY);
    DrawText(gestion->rechercheTexte, rechercheRect.x + 10, rechercheRect.y + 5, 20, BLACK);
    
    if (CheckCollisionPointRec(GetMousePosition(), rechercheRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gestion->saisieActive = 5;
    } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), rechercheRect)) {
        if (gestion->saisieActive == 5) gestion->saisieActive = 0;
    }
    
    GererSaisieTexte(gestion->rechercheTexte, 50, &rechercheActive, false);
    
    if (Bouton(x + 460, y, 100, 30, "Effacer", DARKGRAY, GRAY)) {
        memset(gestion->rechercheTexte, 0, 50);
    }
}

void DessinerEcranProduits(GestionCommerciale *gestion) {
    DrawText("GESTION DES PRODUITS", 40, 100, 30, DARKBLUE);
    
    // Afficher la barre de recherche
    if (gestion->showRechercheBar) {
        DessinerBarreRecherche(gestion);
    }
    
    // Boutons pour la recherche et le tri
    if (Bouton(GetScreenWidth() - 350, 100, 150, 40, gestion->showRechercheBar ? "Masquer recherche" : "Rechercher", DARKBLUE, BLUE)) {
        gestion->showRechercheBar = !gestion->showRechercheBar;
        if (!gestion->showRechercheBar) {
            memset(gestion->rechercheTexte, 0, 50);
        }
    }
    
    // Bouton d'ajout
    if (Bouton(GetScreenWidth() - 170, 100, 150, 40, "Ajouter", GREEN, LIME)) {
        gestion->showAjoutProduit = true;
        memset(gestion->nouveauNom, 0, 50);
        memset(gestion->nouveauPrix, 0, 20);
        memset(gestion->nouveauStock, 0, 20);
        gestion->saisieActive = 0;
    }
    
    // Entête du tableau
    int startY = 170;
    if (gestion->showRechercheBar) startY = 200;
    
    DrawRectangle(40, startY, GetScreenWidth() - 80, 40, LIGHTGRAY);
    
    // Boutons de tri
    if (Bouton(40, startY, 300, 40, "Nom", gestion->trierParNom ? BLUE : LIGHTGRAY, gestion->trierParNom ? DARKBLUE : GRAY)) {
        gestion->trierParNom = true;
        gestion->trierParPrix = false;
        gestion->trierParStock = false;
        gestion->trierAscendant = !gestion->trierAscendant;
        TrierProduits(gestion);
    }
    
    if (Bouton(340, startY, 150, 40, "Prix", gestion->trierParPrix ? BLUE : LIGHTGRAY, gestion->trierParPrix ? DARKBLUE : GRAY)) {
        gestion->trierParNom = false;
        gestion->trierParPrix = true;
        gestion->trierParStock = false;
        gestion->trierAscendant = !gestion->trierAscendant;
        TrierProduits(gestion);
    }
    
    if (Bouton(490, startY, 150, 40, "Stock", gestion->trierParStock ? BLUE : LIGHTGRAY, gestion->trierParStock ? DARKBLUE : GRAY)) {
        gestion->trierParNom = false;
        gestion->trierParPrix = false;
        gestion->trierParStock = true;
        gestion->trierAscendant = !gestion->trierAscendant;
        TrierProduits(gestion);
    }
    
    DrawText(gestion->trierAscendant ? "▲" : "▼", 
             gestion->trierParNom ? 320 : (gestion->trierParPrix ? 470 : 620), 
             startY + 10, 20, BLACK);
    
    // Affichage des produits
    int itemHeight = 40;
    int visibleItems = 10;
    
    for (int i = 0; i < gestion->nombreProduits; i++) {
        // Filtrer par recherche
        if (strlen(gestion->rechercheTexte) > 0) {
            // Vérifier si le nom du produit contient le texte de recherche
            if (strstr(gestion->produits[i].nom, gestion->rechercheTexte) == NULL) {
                continue;
            }
        }
        
        // Alterner les couleurs des lignes
        Color rowColor = (i % 2 == 0) ? WHITE : ColorAlpha(LIGHTGRAY, 0.3f);
        DrawRectangle(40, startY + 40 + i*itemHeight, GetScreenWidth() - 80, itemHeight, rowColor);
        
        // Nom du produit
        DrawText(gestion->produits[i].nom, 50, startY + 50 + i*itemHeight, 20, BLACK);
        
        // Prix
        DrawText(TextFormat("%.2f €", gestion->produits[i].prix), 350, startY + 50 + i*itemHeight, 20, BLACK);
        
        // Stock
        Color stockColor = (gestion->produits[i].stock <= 5) ? RED : 
                          (gestion->produits[i].stock <= 20) ? ORANGE : BLACK;
        DrawText(TextFormat("%d", gestion->produits[i].stock), 500, startY + 50 + i*itemHeight, 20, stockColor);
        
        // Boutons d'action
        if (BoutonIcone(640, startY + 45 + i*itemHeight, 30, "V", GREEN)) {
            gestion->produitSelectionne = i;
            gestion->ecranActuel = 2;
        }
        
        if (BoutonIcone(680, startY + 45 + i*itemHeight, 30, "E", BLUE)) {
            gestion->produitSelectionne = i;
            gestion->showEditProduit = true;
            strcpy(gestion->nouveauNom, gestion->produits[i].nom);
            sprintf(gestion->nouveauPrix, "%.2f", gestion->produits[i].prix);
            sprintf(gestion->nouveauStock, "%d", gestion->produits[i].stock);
            gestion->saisieActive = 0;
        }
        
        if (BoutonIcone(720, startY + 45 + i*itemHeight, 30, "X", RED)) {
            gestion->produitSelectionne = i;
            gestion->showDeleteConfirm = true;
        }
    }
    
    // Aucun produit trouvé
    if (strlen(gestion->rechercheTexte) > 0 && gestion->nombreProduits == 0) {
        DrawText("Aucun produit ne correspond à votre recherche.", 40, startY + 50, 20, GRAY);
    }
}

void DessinerFormulaireProduit(GestionCommerciale *gestion, bool estEdition) {
   // Fond semi-transparent
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
    
    // Fenêtre du formulaire
    int formWidth = 600;
    int formHeight = 400;
    int formX = GetScreenWidth()/2 - formWidth/2;
    int formY = GetScreenHeight()/2 - formHeight/2;
    
    DrawRectangleRounded((Rectangle){formX, formY, formWidth, formHeight}, 0.1f, 10, WHITE);
    // Fix: DrawRectangleRoundedLines has only 4 parameters, not 5
    DrawRectangleRoundedLines((Rectangle){formX, formY, formWidth, formHeight}, 0.1f, 10, DARKBLUE);
    
    // Titre
    DrawText(estEdition ? "MODIFIER LE PRODUIT" : "NOUVEAU PRODUIT", 
             formX + formWidth/2 - MeasureText(estEdition ? "MODIFIER LE PRODUIT" : "NOUVEAU PRODUIT", 30)/2, 
             formY + 20, 30, DARKBLUE);
    // Champ Nom
    DrawText("Nom:", formX + 40, formY + 80, 20, BLACK);
    Rectangle nomRect = {formX + 170, formY + 80, 380, 30};
    bool nomActive = (gestion->saisieActive == 1);
    DrawRectangleRec(nomRect, nomActive ? YELLOW : WHITE);
    DrawRectangleLinesEx(nomRect, 1, DARKGRAY);
    DrawText(gestion->nouveauNom, nomRect.x + 10, nomRect.y + 5, 20, BLACK);
    
    if (CheckCollisionPointRec(GetMousePosition(), nomRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gestion->saisieActive = 1;
    }
    GererSaisieTexte(gestion->nouveauNom, 50, &nomActive, false);
    
    // Champ Prix
    DrawText("Prix:", formX + 40, formY + 130, 20, BLACK);
    Rectangle prixRect = {formX + 170, formY + 130, 380, 30};
    bool prixActive = (gestion->saisieActive == 2);
    DrawRectangleRec(prixRect, prixActive ? YELLOW : WHITE);
    DrawRectangleLinesEx(prixRect, 1, DARKGRAY);
    DrawText(gestion->nouveauPrix, prixRect.x + 10, prixRect.y + 5, 20, BLACK);
    
    if (CheckCollisionPointRec(GetMousePosition(), prixRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gestion->saisieActive = 2;
    }
    GererSaisieTexte(gestion->nouveauPrix, 20, &prixActive, true);
    
    // Champ Stock
    DrawText("Stock:", formX + 40, formY + 180, 20, BLACK);
    Rectangle stockRect = {formX + 170, formY + 180, 380, 30};
    bool stockActive = (gestion->saisieActive == 3);
    DrawRectangleRec(stockRect, stockActive ? YELLOW : WHITE);
    DrawRectangleLinesEx(stockRect, 1, DARKGRAY);
    DrawText(gestion->nouveauStock, stockRect.x + 10, stockRect.y + 5, 20, BLACK);
    
    if (CheckCollisionPointRec(GetMousePosition(), stockRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gestion->saisieActive = 3;
    }
    GererSaisieTexte(gestion->nouveauStock, 20, &stockActive, true);
    
    // Boutons
    if (Bouton(formX + 140, formY + 320, 120, 40, "Valider", GREEN, LIME)) {
        if (estEdition) {
            ModifierProduit(gestion);
        } else {
            AjouterProduit(gestion);
        }
        gestion->showAjoutProduit = false;
        gestion->showEditProduit = false;
    }
    
    if (Bouton(formX + 340, formY + 320, 120, 40, "Annuler", MAROON, RED)) {
        gestion->showAjoutProduit = false;
        gestion->showEditProduit = false;
    }
}
// Function to modify an existing product
void ModifierProduit(GestionCommerciale *gestion) {
    if (gestion->produitSelectionne >= 0 && gestion->produitSelectionne < gestion->nombreProduits) {
        // Update product information
        strncpy(gestion->produits[gestion->produitSelectionne].nom, gestion->nouveauNom, 49);
        gestion->produits[gestion->produitSelectionne].prix = atof(gestion->nouveauPrix);
        gestion->produits[gestion->produitSelectionne].stock = atoi(gestion->nouveauStock);
        
        // Show a confirmation notification
        AfficherNotification(gestion, "Produit modifié avec succès");
    }
}

// Function to add a new product
void AjouterProduit(GestionCommerciale *gestion) {
    if (gestion->nombreProduits < MAX_PRODUITS) {
        Produit nouveau;
        strncpy(nouveau.nom, gestion->nouveauNom, 49);
        nouveau.prix = atof(gestion->nouveauPrix);
        nouveau.stock = atoi(gestion->nouveauStock);
        nouveau.id = gestion->nombreProduits + 1; // Auto-increment ID
        
        gestion->produits[gestion->nombreProduits] = nouveau;
        gestion->nombreProduits++;
        
        // Show a confirmation notification
        AfficherNotification(gestion, "Produit ajouté avec succès");
    } else {
        // Show an error notification if maximum products reached
        AfficherNotification(gestion, "Nombre maximum de produits atteint");
    }
}

// Function to delete a product
void SupprimerProduit(GestionCommerciale *gestion, int index) {
    if (index >= 0 && index < gestion->nombreProduits) {
        // Shift all products after the deleted one
        for (int i = index; i < gestion->nombreProduits - 1; i++) {
            gestion->produits[i] = gestion->produits[i + 1];
        }
        gestion->nombreProduits--;
    }
}

// Function to display the sales screen
void DessinerEcranVentes(GestionCommerciale *gestion) {
    DrawText("ENREGISTRER UNE VENTE", 40, 100, 30, DARKBLUE);
    
    if (gestion->produitSelectionne >= 0 && gestion->produitSelectionne < gestion->nombreProduits) {
        Produit p = gestion->produits[gestion->produitSelectionne];
        
        // Display product information
        DrawRectangleRounded((Rectangle){40, 150, 500, 200}, 0.1f, 10, ColorAlpha(LIGHTGRAY, 0.3f));
        DrawText(TextFormat("Produit: %s", p.nom), 60, 170, 22, BLACK);
        DrawText(TextFormat("Prix unitaire: %.2f €", p.prix), 60, 210, 22, BLACK);
        DrawText(TextFormat("Stock disponible: %d", p.stock), 60, 250, 22, BLACK);
        
        // Quantity input field
        static char quantiteStr[10] = "1";
        DrawText("Quantité:", 60, 290, 22, BLACK);
        Rectangle quantiteRect = {180, 290, 100, 30};
        bool quantiteActive = (gestion->saisieActive == 4);
        DrawRectangleRec(quantiteRect, quantiteActive ? YELLOW : WHITE);
        DrawRectangleLinesEx(quantiteRect, 1, DARKGRAY);
        DrawText(quantiteStr, 190, 295, 20, BLACK);
        
        if (CheckCollisionPointRec(GetMousePosition(), quantiteRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            gestion->saisieActive = 4;
        }
        GererSaisieTexte(quantiteStr, 10, &quantiteActive, true);
        
        // Calculate total price
        int quantite = atoi(quantiteStr);
        if (quantite > 0) {
            DrawText(TextFormat("Prix total: %.2f €", p.prix * quantite), 60, 330, 22, DARKBLUE);
        }
        
        // Sale validation button
        if (Bouton(60, 370, 200, 40, "Valider la vente", GREEN, LIME)) {
            int quantite = atoi(quantiteStr);
            if (quantite > 0 && quantite <= p.stock) {
                EnregistrerVente(gestion, gestion->produitSelectionne, quantite);
                gestion->produits[gestion->produitSelectionne].stock -= quantite;
                quantiteStr[0] = '1';
                quantiteStr[1] = '\0';
                AfficherNotification(gestion, "Vente enregistrée avec succès");
                gestion->produitSelectionne = -1;
            } else if (quantite > p.stock) {
                AfficherNotification(gestion, "Stock insuffisant");
            }
        }
    } else {
        // Display message if no product is selected
        DrawText("Veuillez sélectionner un produit à vendre", 40, 170, 24, GRAY);
    }
    
    // Button to select a product if none is selected
    if (Bouton(GetScreenWidth() - 250, 100, 200, 40, "Choisir un produit", DARKBLUE, BLUE)) {
        gestion->ecranActuel = 1;
    }
}

// Function to display the sales history screen
void DessinerEcranHistoriqueVentes(GestionCommerciale *gestion) {
    DrawText("HISTORIQUE DES VENTES", 40, 100, 30, DARKBLUE);
    
    int startY = 170;
    int itemsPerPage = 12;
    int startIndex = gestion->ventePage * itemsPerPage;
    int endIndex = startIndex + itemsPerPage;
    if (endIndex > gestion->nombreVentes) endIndex = gestion->nombreVentes;
    
    // Table headers
    DrawRectangle(40, startY, GetScreenWidth() - 80, 40, LIGHTGRAY);
    DrawText("Date", 60, startY + 10, 20, BLACK);
    DrawText("Produit", 250, startY + 10, 20, BLACK);
    DrawText("Quantité", 500, startY + 10, 20, BLACK);
    DrawText("Prix Unit.", 600, startY + 10, 20, BLACK);
    DrawText("Total", 720, startY + 10, 20, BLACK);
    
    // Display sales
    if (gestion->nombreVentes == 0) {
        DrawText("Aucune vente enregistrée", 40, startY + 60, 20, GRAY);
    } else {
        for (int i = startIndex; i < endIndex; i++) {
            Vente v = gestion->ventes[i];
            Color rowColor = (i % 2 == 0) ? WHITE : ColorAlpha(LIGHTGRAY, 0.3f);
            int rowY = startY + 40 + (i - startIndex) * 40;
            
            DrawRectangle(40, rowY, GetScreenWidth() - 80, 40, rowColor);
            
            // Format date
            char dateStr[30];
            struct tm *timeinfo = localtime(&v.date);
            strftime(dateStr, 30, "%d/%m/%Y %H:%M", timeinfo);
            
            DrawText(dateStr, 60, rowY + 10, 20, BLACK);
            DrawText(v.produit.nom, 250, rowY + 10, 20, BLACK);
            DrawText(TextFormat("%d", v.quantite), 500, rowY + 10, 20, BLACK);
            DrawText(TextFormat("%.2f €", v.produit.prix), 600, rowY + 10, 20, BLACK);
            DrawText(TextFormat("%.2f €", v.quantite * v.produit.prix), 720, rowY + 10, 20, BLACK);
        }
        
        // Pagination
        int totalPages = (gestion->nombreVentes + itemsPerPage - 1) / itemsPerPage;
        DrawText(TextFormat("Page %d/%d", gestion->ventePage + 1, totalPages), GetScreenWidth()/2 - 40, startY + 520, 20, DARKGRAY);
        
        if (gestion->ventePage > 0) {
            if (Bouton(GetScreenWidth()/2 - 100, startY + 520, 40, 30, "<", BLUE, DARKBLUE)) {
                gestion->ventePage--;
            }
        }
        
        if (gestion->ventePage < totalPages - 1) {
            if (Bouton(GetScreenWidth()/2 + 60, startY + 520, 40, 30, ">", BLUE, DARKBLUE)) {
                gestion->ventePage++;
            }
        }
    }
}

// Function to display the statistics screen
void DessinerEcranStats(GestionCommerciale *gestion) {
    DrawText("STATISTIQUES DE VENTES", 40, 100, 30, DARKBLUE);
    
    // Global statistics
    int totalVentes = 0;
    float totalCA = 0.0f;
    float meilleureVente = 0.0f;
    char produitMeilleurVente[50] = "";
    int meilleureQuantite = 0;
    
    for (int i = 0; i < gestion->nombreVentes; i++) {
        float montantVente = gestion->ventes[i].quantite * gestion->ventes[i].produit.prix;
        totalVentes += gestion->ventes[i].quantite;
        totalCA += montantVente;
        
        if (montantVente > meilleureVente) {
            meilleureVente = montantVente;
            strcpy(produitMeilleurVente, gestion->ventes[i].produit.nom);
            meilleureQuantite = gestion->ventes[i].quantite;
        }
    }
    
    // Display global statistics
    DrawRectangleRounded((Rectangle){40, 150, 350, 180}, 0.1f, 10, ColorAlpha(LIGHTGRAY, 0.3f));
    DrawText("RÉSUMÉ GÉNÉRAL", 60, 160, 20, DARKBLUE);
    DrawText(TextFormat("Nombre total de ventes: %d", gestion->nombreVentes), 60, 190, 20, BLACK);
    DrawText(TextFormat("Quantité totale vendue: %d", totalVentes), 60, 220, 20, BLACK);
    DrawText(TextFormat("Chiffre d'affaires: %.2f €", totalCA), 60, 250, 20, BLACK);
    if (gestion->nombreVentes > 0) {
        DrawText(TextFormat("Panier moyen: %.2f €", totalCA / gestion->nombreVentes), 60, 280, 20, BLACK);
        DrawText(TextFormat("Meilleure vente: %s (%d)", produitMeilleurVente, meilleureQuantite), 60, 310, 20, BLACK);
    }
    
    // Time period selector
    DrawText("Période:", 420, 160, 20, DARKBLUE);
    if (Bouton(500, 160, 100, 30, "Jour", gestion->statsVue == 0 ? BLUE : DARKGRAY, gestion->statsVue == 0 ? DARKBLUE : GRAY)) {
        gestion->statsVue = 0;
    }
    if (Bouton(610, 160, 100, 30, "Semaine", gestion->statsVue == 1 ? BLUE : DARKGRAY, gestion->statsVue == 1 ? DARKBLUE : GRAY)) {
        gestion->statsVue = 1;
    }
    if (Bouton(720, 160, 100, 30, "Mois", gestion->statsVue == 2 ? BLUE : DARKGRAY, gestion->statsVue == 2 ? DARKBLUE : GRAY)) {
        gestion->statsVue = 2;
    }
    
    // Determine period for graph
    int periode = 0;
    const char *periodeLabel = "";
    switch (gestion->statsVue) {
        case 0: periode = 24; periodeLabel = "Heures"; break;  // 24 hours
        case 1: periode = 7; periodeLabel = "Jours"; break;    // 7 days
        case 2: periode = 30; periodeLabel = "Jours"; break;   // 30 days
    }
    
    // Sales data for the selected period
    float ventesParPeriode[30] = {0};  // Max 30 days
    float caParPeriode[30] = {0};
    time_t maintenant = time(NULL);
    
    // Calculate sales and revenue for each period
    for (int i = 0; i < gestion->nombreVentes; i++) {
        int periodeIndex = 0;
        
        if (gestion->statsVue == 0) {  // Hours
            double diffSec = difftime(maintenant, gestion->ventes[i].date);
            periodeIndex = (int)(diffSec / 3600);  // Convert to hours
        } else {  // Days
            double diffSec = difftime(maintenant, gestion->ventes[i].date);
            periodeIndex = (int)(diffSec / (24 * 3600));  // Convert to days
        }
        
        if (periodeIndex < periode) {
            ventesParPeriode[periodeIndex] += gestion->ventes[i].quantite;
            caParPeriode[periodeIndex] += gestion->ventes[i].quantite * gestion->ventes[i].produit.prix;
        }
    }
    
    // Draw sales graph
    int graphX = 420;
    int graphY = 370;
    int graphWidth = 800;
    int graphHeight = 180;
    
    DrawRectangleRounded((Rectangle){graphX, graphY - 20, graphWidth, graphHeight + 40}, 0.1f, 10, ColorAlpha(LIGHTGRAY, 0.1f));
    DrawText(TextFormat("Ventes par %s", periodeLabel), graphX + 10, graphY - 10, 20, DARKBLUE);
    
    // Find max value for scale
    float maxVentes = 1.0f;  // Minimum to avoid division by zero
    for (int i = 0; i < periode; i++) {
        if (ventesParPeriode[i] > maxVentes) maxVentes = ventesParPeriode[i];
    }
    
    // Draw bars
    int barWidth = (graphWidth - 40) / periode;
    int barSpacing = 2;
    
    for (int i = 0; i < periode; i++) {
        int barHeight = (int)((ventesParPeriode[periode - i - 1] / maxVentes) * graphHeight);
        if (barHeight < 0) barHeight = 0;
        
        DrawRectangle(
            graphX + 20 + i * (barWidth + barSpacing),
            graphY + graphHeight - barHeight,
            barWidth,
            barHeight,
            BLUE
        );
        
        // Period label
        if (gestion->statsVue == 0) {  // Hours
            DrawText(TextFormat("%dh", periode - i - 1), 
                    graphX + 20 + i*(barWidth + barSpacing) + barWidth/2 - 10, 
                    graphY + graphHeight + 10, 10, BLACK);
        } else {  // Days
            DrawText(TextFormat("J-%d", periode - i - 1), 
                    graphX + 20 + i*(barWidth + barSpacing) + barWidth/2 - 10, 
                    graphY + graphHeight + 10, 10, BLACK);
        }
        
        // Value label (only show if bar has height)
        if (barHeight > 20) {
            DrawText(TextFormat("%.0f", ventesParPeriode[periode - i - 1]), 
                    graphX + 20 + i*(barWidth + barSpacing) + barWidth/2 - 10, 
                    graphY + graphHeight - barHeight - 15, 10, WHITE);
        }
    }
    
    // Draw stock status
    DrawText("ÉTAT DES STOCKS", 40, 350, 20, DARKBLUE);
    
    int critiques = 0;
    int faibles = 0;
    int normaux = 0;
    
    for (int i = 0; i < gestion->nombreProduits; i++) {
        if (gestion->produits[i].stock <= 3) critiques++;
        else if (gestion->produits[i].stock <= 10) faibles++;
        else normaux++;
    }
    
    DrawRectangleRounded((Rectangle){40, 380, 350, 130}, 0.1f, 10, ColorAlpha(LIGHTGRAY, 0.3f));
    DrawText(TextFormat("Stock critique (≤ 3): %d produits", critiques), 60, 400, 20, RED);
    DrawText(TextFormat("Stock faible (≤ 10): %d produits", faibles), 60, 430, 20, ORANGE);
    DrawText(TextFormat("Stock normal: %d produits", normaux), 60, 460, 20, DARKGREEN);
    DrawText(TextFormat("Total: %d produits", gestion->nombreProduits), 60, 490, 20, BLACK);
}


// Function to record a sale
void EnregistrerVente(GestionCommerciale *gestion, int idProduit, int quantite) {
    if (gestion->nombreVentes < MAX_VENTES && idProduit >= 0 && idProduit < gestion->nombreProduits) {
        Vente nouvelleVente;
        
        // Copy product information
        nouvelleVente.produit = gestion->produits[idProduit];
        
        // Set quantity and date
        nouvelleVente.quantite = quantite;
        nouvelleVente.date = time(NULL);  // Current timestamp
        
        // Calculate total price
        nouvelleVente.prixTotal = nouvelleVente.produit.prix * quantite;
        
        // Add to sales array
        gestion->ventes[gestion->nombreVentes] = nouvelleVente;
        gestion->nombreVentes++;
    }
}
// Fix for the DessinerConfirmationSuppression function
void DessinerConfirmationSuppression(GestionCommerciale *gestion) {
    // Fond semi-transparent
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
    
    // Fenêtre de confirmation
    int formWidth = 400;
    int formHeight = 200;
    int formX = GetScreenWidth()/2 - formWidth/2;
    int formY = GetScreenHeight()/2 - formHeight/2;
    
    DrawRectangleRounded((Rectangle){formX, formY, formWidth, formHeight}, 0.1f, 10, WHITE);
    // Fix: DrawRectangleRoundedLines has only 4 parameters, not 5
    DrawRectangleRoundedLines((Rectangle){formX, formY, formWidth, formHeight}, 0.1f, 10, RED);
    
    // Message
    DrawText("CONFIRMATION", formX + formWidth/2 - MeasureText("CONFIRMATION", 30)/2, formY + 30, 30, RED);
    DrawText("Voulez-vous vraiment supprimer ce produit ?", 
             formX + formWidth/2 - MeasureText("Voulez-vous vraiment supprimer ce produit ?", 20)/2, 
             formY + 80, 20, BLACK);
    DrawText(gestion->produits[gestion->produitSelectionne].nom, 
             formX + formWidth/2 - MeasureText(gestion->produits[gestion->produitSelectionne].nom, 20)/2, 
             formY + 110, 20, BLACK);  // Fixed: added the missing color parameter and completed the line
             
    // Boutons
    if (Bouton(formX + 100, formY + 150, 120, 30, "Confirmer", RED, MAROON)) {
        SupprimerProduit(gestion, gestion->produitSelectionne);
        gestion->showDeleteConfirm = false;
        AfficherNotification(gestion, "Produit supprimé avec succès");
    }
    
    if (Bouton(formX + 240, formY + 150, 120, 30, "Annuler", DARKGRAY, GRAY)) {
        gestion->showDeleteConfirm = false;
    }
}