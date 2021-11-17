/*******************************************************************************************
*
*   Lagerberekening v1.0.0 - Tool Description
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2021 Pepijn. All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/
/**
 * malloc veranderen door realloc
 * Veranderlijke belasting ingevulde gegevens nog aanpassen naar realloc in plaats van initialiseren met 100
 * https://github.com/oz123/awesome-c
 * Algemene naamgeving verbeteren
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "lagerfuncties.h"


//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------

static void centrifugaalbelasting_btnBerekenen(double statisch, double dynamisch, int lagertype, double* resultaat);

static void veranderlijkeBelasting_btnToevoegen(double tijdstip, double toerental, double belasting);

static void veranderlijkeBelasting_btnReset(void);

static void veranderlijkeBelasting_btnBereken(char* txtEquivalenteBelasting, char* txtGemiddeldToerental, int gekozenlager, bool aisoberekenen, double referentieviscositeit, double temperatuur, double fatigueloadlimit, double properheid, double gemiddeldediameter);

static void lagergegevens_btnZoeken(char *zoekterm, char *lijst, gevondenlagers *delagers);

static void equivalenteBelasting_btnBerekenen(lagerinformatie lager, double radiaalkracht, double axiaalkracht, char* resultaat);

static void grafiekTekenen(void);

static bool GuiValueBox_double(Rectangle bounds, char* text, int textsize, bool editMode, double* value, char* oldText, double minValue, double maxValue);

// Alle informatie die wordt gebruikt voor één textvak
struct textvak
{
    bool EditMode;
    char* Text; // Moet 32 zijn
};

static struct textvak** textvakkenMaken(int rij, int kolom);

static void textvakkenVerwijderen(struct textvak **vakken, int rij, int kolom);

static struct textvak** TekstvakRijToevoegen(struct textvak** oudtextvak, int oudaantalrijen, int oudaantalkolommen);

static void GuiTabel(int locatieX, int locatieY, int rijen, int kolommen, int breedtetextvak, int hoogtetextvak, struct textvak** vakken);


//----------------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------------

// Gegevens veranderlijke belasting
static struct textvak** veranderlijkeBelasting_Tabelvakken; // [rij][kolom](gegevenstype = tijd; toerental; belastig)
static int veranderlijkeBelasting_Rijen = 0;
static int veranderlijkeBelasting_Kolommen = 3;
// [rij][gegevenstype = tijd; toerental; belastig]
static struct veranderlijkeBelasting_procesgegevens veranderlijkeBelasting_IngevuldeGegevens[100]; // Waarschijnlijk zullen er nooit meer dan 100 rijen worden ingevuld. Kan ook met malloc gebeuren, maar dat is te veel werk.

// tabel lagergegevens
static struct textvak** lagergegevens_Tabelvakken;
static int lagergegevens_Rijen = 0;
static int lagergegevens_kolommen = 2;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 1500;
    int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Lagerberekening");

    // Lagerberekening: controls initialization
    //----------------------------------------------------------------------------------
    // Define controls variables

    // Lagerlevensduur
    // DropdownBox: Lagertype
    bool levensduur_ddbLagerTypeEditMode = false;
    int levensduur_ddbLagerTypeActive = 0;            // DropdownBox: ddbLagerType
    // ValueBox_double: Dynamisch draaggetal
    bool levensduur_vbDynamischDraaggetalEditMode = false;
    double levensduur_vbDynamischDraaggetalValue = 0.0;            // ValueBOx: vbDynamischDraaggetal
    char levensduur_vbDynamischDraaggetalText[32] = "0";
    char levensduur_oud_vbDynamischDraaggetalText[32] = "0";
    // ValueBox_double: Equivalente belasting
    bool levensduur_vbEquivalenteBelastingEditMode = false;
    double levensduur_vbEquivalenteBelastingValue = 0.0;            // ValueBOx: vbEquivalenteBelasting
    char levensduur_vbEquivalenteBelastingText[32] = "0";
    char levensduur_oud_vbEquivalenteBelastingText[32] = "0";
    // ValueBox_double: Toerental
    bool levensduur_vbToerentalEditMode = false;
    double levensduur_vbToerentalValue = 0.0;            // ValueBOx: vbToerentalLevensduur
    char levensduur_vbToerentalText[32] = "0";
    char levensduur_oud_vbToerentalText[32] = "0";
    // Combobox tijdsbasis
    int levensduur_cmbTijdsbasisActive = 0;            // ComboBox: cmbTijdsbasis
    // TextBox: Standaard Levensduur
    bool levensduur_txtStandaardLevensduurEditMode = false;
    char levensduur_txtStandaardLevensduurText[32] = "0";            // TextBox: txtStandaardLevensduur
    // DropDownBox betrouwbaarheid
    bool levensduur_ddbBetrouwbaarheidEditMode = false;
    int levensduur_ddbBetrouwbaarheidActive = 0;            // DropdownBox: ddbBetrouwbaarheid
    // ValueBox_double: Fatigue Load limit
    bool levensduur_vbFatigueLoadLimitEditMode = false;
    double levensduur_vbFatigueLoadLimitValue = 0;            // ValueBOx: vbFatigueLoadLimit
    char levensduur_vbFatigueLoadLimitText[32] = "0";
    char levensduur_oud_vbFatigueLoadLimitText[32] = "0";
    // ValueBox_double: Properheid
    bool levensduur_vbProperheidEditMode = false;
    double levensduur_vbProperheidValue = 0.0;            // ValueBOx: vbProperheid
    char levensduur_vbProperheidText[32] = "0";
    char levensduur_oud_vbProperheidText[32] = "0";
    // ValueBox_double: Smeringsverhouding
    bool levensduur_vbSmeringsverhoudingEditMode = false;
    double levensduur_vbSmeringsverhoudingValue = 0;            // ValueBOx: vbSmeringsverhouding
    char levensduur_vbSmeringsverhoudingText[32] = "0";
    char levensduur_oud_vbSmeringsverhoudingText[32] = "0";
    // Textbox: Aangepaste Levensduur
    bool levensduur_txtAangepasteLevensduurEditMode = false;
    char levensduur_txtAangepasteLevensduurText[32] = "0";            // TextBox: txtAangepasteLevensduur
    
    // Smering
    // ValueBox: Buitendiameter
    bool smering_vbBuitendiameterEditMode = false;
    double smering_vbBuitendiameterValue = 0.0;            // ValueBOx: vbBuitendiameter
    char smering_vbBuitendiameterText[32] = "0";
    char smering_oud_vbBuitendiameterText[32] = "0";
    // ValueBox: Binnendiameter
    bool smering_vbBinnendiameterEditMode = false;
    double smering_vbBinnendiameterValue = 0.0;            // ValueBOx: vbBinnendiameter
    char smering_vbBinnendiameterText[32] = "0";
    char smering_oud_vbBinnendiameterText[32] = "0";
    // Textbox: Gemiddelde Diameter
    bool smering_txtGemiddeldeDiameterEditMode = false;
    char smering_txtGemiddeldeDiameterText[32] = "0";            // TextBox: txtGemiddeldeDiameter
    // ValueBox: Toerental
    bool smering_vbToerentalSmeringEditMode = false;
    double smering_vbToerentalSmeringValue = 0.0;            // ValueBOx: vbToerentalSmering
    char smering_vbToerentalSmerintText[32] = "0";
    char smering_oud_vbToerentalSmerintText[32] = "0";
    // TextBox: Nodige Viscositeit
    bool smering_txtNodigeViscositeitEditMode = false;
    char smering_txtNodigeViscositeitText[32] = "0";            // TextBox: txtNodigeViscositeit
    // ValueBox: Gebruikte Viscositeit
    bool smering_vbGebruikteViscositeitEditMode = false;
    double smering_vbGebruikteViscositeitValue = 0;            // ValueBOx: vbGebruikteViscositeit
    char smering_vbGebruikteViscositeitText[32] = "0";
    char smering_oud_vbGebruikteViscositeitText[32] = "0";
    // Valuebox: Werkingstemperatur
    bool smering_vbWerkingsTemperatuurEditMode = false;
    double smering_vbWerkingsTemperatuurValue = 0;            // ValueBOx: vbWerkingsTemperatuur
    char smering_vbWerkingsTemperatuurText[32] = "0";
    char smering_oud_vbWerkingsTemperatuurText[32] = "0";
    // Textbox: Effectieve viscositeit
    bool smering_txtEffectieveViscositeitEditMode = false;
    char smering_txtEffectieveViscositeitText[32] = "0";
    // TextBox: Smeringsverhouding
    bool smering_txtBerekendeSmeringsverhoudingEditMode = false;
    char smering_txtBerekendeSmeringsverhoudingText[32] = "0";            // TextBox: txtBerekendeSmeringsverhouding


    // samengestelde centrifugaalbelasting
    // Toggle Group: Lagertype
    int centrifugaalbelasting_tgLagertypeActive = 0;
    // ValueBox:  onbalansmassa
    bool centrifugaalbelasting_vbOnbalansmassaEditMode = false;
    double centrifugaalbelasting_vbOnbalansmassaValue = 0;            // ValueBOx: vbOnbalansmassa
    char centrifugaalbelasting_vbOnbalansmassaText[32] = "0";
    char centrifugaalbelasting_oud_vbOnbalansmassaText[32] = "0";
    // ValueBox:  straal
    bool centrifugaalbelasting_vbStraalEditMode = false;
    double centrifugaalbelasting_vbStraalValue = 0;            // ValueBOx: vbstraal
    char centrifugaalbelasting_vbStraalText[32] = "0";
    char centrifugaalbelasting_oud_vbStraalText[32] = "0";
    // ValueBox:  Toerental
    bool centrifugaalbelasting_vbToerentalEditMode = false;
    double centrifugaalbelasting_vbToerentalValue = 0;            // ValueBOx: vbToerental
    char centrifugaalbelasting_vbToerentalText[32] = "0";
    char centrifugaalbelasting_oud_vbToerentalText[32] = "0";
    // Textbox: OnbalanskrachtBerekend
    bool centrifugaalbelasting_txtOnbalanskrachtBerekendEditMode = false;
    char centrifugaalbelasting_txtOnbalanskrachtBerekendText[32] = "0";
    // ValueBox:  Onbalanskracht_Gekozen
    bool centrifugaalbelasting_vbOnbalanskrachtGekozenEditMode = false;
    double centrifugaalbelasting_vbOnbalanskrachtGekozenValue = 0;            // ValueBOx: vbToerental
    char centrifugaalbelasting_vbOnbalanskrachtGekozenText[32] = "0";
    char centrifugaalbelasting_oud_vbOnbalanskrachtGekozenText[32] = "0";
    // ValueBox:  Onbalanskracht_Gekozen
    bool centrifugaalbelasting_vbStatischeKrachtEditMode = false;
    double centrifugaalbelasting_vbStatischeKrachtValue = 0;            // ValueBOx: vbToerental
    char centrifugaalbelasting_vbStatischeKrachtText[32] = "0";
    char centrifugaalbelasting_oud_vbStatischeKrachtText[32] = "0";
    // Textbox: OnbalanskrachtBerekend
    bool centrifugaalbelasting_txtEquivalenteBelastingEditMode = false;
    char centrifugaalbelasting_txtEquivalenteBelastingText[32] = "0";
    

    // Veranderlijke belasting
    // Combobox LagerType
    int veranderlijkeBelasting_cmbLagerTypeActive = 0;            // ComboBox: cmbLagerType
    // CheckBox Aiso berekenen
    bool veranderlijkeBelasting_cbAisoFactor = false;
    // ValueBox:  Viscositeit
    bool veranderlijkeBelasting_vbViscositeitEditMode = false;
    double veranderlijkeBelasting_vbViscositeitValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbViscositeitText[32] = "0";
    char veranderlijkeBelasting_oud_vbViscositeitText[32] = "0";
    // ValueBox:  Temperatuur
    bool veranderlijkeBelasting_vbTemperatuurEditMode = false;
    double veranderlijkeBelasting_vbTemperatuurValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbTemperatuurText[32] = "0";
    char veranderlijkeBelasting_oud_vbTemperatuurText[32] = "0";
    // ValueBox:  Fatigue Load Limit
    bool veranderlijkeBelasting_vbFatigueEditMode = false;
    double veranderlijkeBelasting_vbFatigueValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbFatigueText[32] = "0";
    char veranderlijkeBelasting_oud_vbFatigueText[32] = "0";
    // ValueBox:  Properheid
    bool veranderlijkeBelasting_vbProperheidEditMode = false;
    double veranderlijkeBelasting_vbProperheidValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbProperheidText[32] = "0";
    char veranderlijkeBelasting_oud_vbProperheidText[32] = "0";
    // ValueBox:  tijdstip
    bool veranderlijkeBelasting_vbTijdstipEditMode = false;
    double veranderlijkeBelasting_vbTijdstipValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbTijdstipText[32] = "0";
    char veranderlijkeBelasting_oud_vbTijdstipText[32] = "0";
    // ValueBox:  Toerental
    bool veranderlijkeBelasting_vbToerentalEditMode = false;
    double veranderlijkeBelasting_vbToerentalValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbToerentalText[32] = "0";
    char veranderlijkeBelasting_oud_vbToerentalText[32] = "0";
    // ValueBox:  Belasting
    bool veranderlijkeBelasting_vbBelastingEditMode = false;
    double veranderlijkeBelasting_vbBelastingValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbBelastingText[32] = "0";
    char veranderlijkeBelasting_oud_vbBelastingText[32] = "0";
    // ValueBox:  Gemiddelde Diameter
    bool veranderlijkeBelasting_vbGemiddeldeDiameterEditMode = false;
    double veranderlijkeBelasting_vbGemiddeldeDiameterValue = 0;            // ValueBOx: vbToerental
    char veranderlijkeBelasting_vbGemiddeldeDiameterText[32] = "0";
    char veranderlijkeBelasting_oud_vbGemiddeldeDiameterText[32] = "0";
    // Textbox: Equivalente belasting
    bool veranderlijkeBelasting_txtEquivalenteBelastingEditMode = false;
    char veranderlijkeBelasting_txtEquivalenteBelastingText[32] = "0";
    // Textbox: Gemiddeld Toerental
    bool veranderlijkeBelasting_txtGemiddeldToerentalEditMode = false;
    char veranderlijkeBelasting_txtGemiddeldToerentalText[32] = "0";


    // Lagergegevens:
    // Textbox: lagercode
    bool lagergegevens_txtLagercodeEditMode = false;
    char lagergegevens_txtLagercodeText[32] = "";
    // Dropdownbox: gevonden lagers
    bool lagergegevens_ddbGevondenLagerEditMode = false;
    bool lagergegevens_ddbGevondenLagerFlankbit = false;
    int lagergegevens_ddbGevondenLagerActive = 0; 
    char lagergegevens_ddbGevondenLagerText[1024] = "";
    // Label: type lager
    char lagergegevens_lblTypeLager[32] = {0};
    
    // Equivalente belasting
    // ValueBox:  Radiale kracht
    bool equivalenteBelasting_vbRadialeKrachtEditMode = false;
    double equivalenteBelasting_vbRadialeKrachtValue = 0;            // ValueBOx: vbToerental
    char equivalenteBelasting_vbRadialeKrachtText[32] = "0";
    char equivalenteBelasting_oud_vbRadialeKrachtText[32] = "0";
    // ValueBox:  Axiale kracht
    bool equivalenteBelasting_vbAxialeKrachtEditMode = false;
    double equivalenteBelasting_vbAxialeKrachtValue = 0;            // ValueBOx: vbToerental
    char equivalenteBelasting_vbAxialeKrachtText[32] = "0";
    char equivalenteBelasting_oud_vbAxialeKrachtText[32] = "0";
    // TextBox: EquivalenteBelasting
    char equivalenteBelasting_txtEquivalenteBelastingText[32] = "0";



    // Bij te houden gegevens
    gevondenlagers lagergegevensGevondenLagers;
    lagergegevensGevondenLagers.aantal = 0;
    lagergegevensGevondenLagers.lagers = NULL;
    lagerinformatie lagergegevensGekozenLager;
    lagergegevensGekozenLager.aantalGegevens = 0;
    lagergegevensGekozenLager.kolomtitels = NULL;
    lagergegevensGekozenLager.lagergegevens = NULL;
    lagergegevensGekozenLager.lagersoort = 0;

    // Berekende Waarden
    double levensduur_Berekend = 0.0;
    double gemiddeldeDiameter_Berekend = 0;
    double nodigeViscositeit_Berekend = 0;
    double effectieveViscositeit_Berekend = 0;
    double kWaarde = 0;
    double onbalanskracht_Berekend = 0;
    double equivalenteSamengesteldeCentrifugaalBelasting = 0;


    // Oude waarden om te controleren of er iets is veranderd
    double oud_levensduur_Berekend = 0.0;
    double oud_gemiddeldeDiameter_Berekend = 0;
    double oud_nodigeViscositeit_Berekend = 0;
    double oud_effectieveViscositeit_Berekend = 0;
    double oud_equivalenteSamengesteldeCentrifugaalBelasting = 0;
    
    //----------------------------------------------------------------------------------

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Worden indien er een vak is aangeduid continu berekend, maar waarschijnlijk is de berekening niet veel belastender dan het continu controleren of er iets is gewijzigd.
        // Berekenen van standaard levensduur
        if (levensduur_ddbLagerTypeActive || levensduur_cmbTijdsbasisActive || levensduur_vbDynamischDraaggetalEditMode || levensduur_vbEquivalenteBelastingEditMode || levensduur_vbToerentalEditMode)
        {
            if ((levensduur_vbToerentalValue > 0) && (levensduur_vbEquivalenteBelastingValue > 0) && (levensduur_vbDynamischDraaggetalValue > 0))
            {
                // Indien de waarden zijn ingevuld, kan de levenduur worden berekend.
                // Bepalen welk type lager er is gekozen
                double exponent;
                if (levensduur_ddbLagerTypeActive == 1 || levensduur_ddbLagerTypeActive == 3) // Radiaal Rollager Axiaal Rollager
                {
                    exponent = 10.0 / 3.0;
                }
                else // Radiaal Kogellager Axiaal Kogellager
                {
                    exponent = 3.0;
                }

                // Levensduur in miljoen omwentelingen
                levensduur_Berekend = standaardLevensduur((double)levensduur_vbDynamischDraaggetalValue, (double)levensduur_vbEquivalenteBelastingValue, exponent);

                // Controleren of er geen andere tijdsbasis nodig is.

                switch (levensduur_cmbTijdsbasisActive) // 0 = OMwentelingen
                {
                case 1: // Uren
                    levensduur_Berekend = levensduurOpTijd(levensduur_Berekend, levensduur_vbToerentalValue, UUR);
                    break;
                case 2:
                    levensduur_Berekend = levensduurOpTijd(levensduur_Berekend, levensduur_vbToerentalValue, DAGEN);
                    break;
                case 3:
                    levensduur_Berekend = levensduurOpTijd(levensduur_Berekend, levensduur_vbToerentalValue, JAREN);
                    break;
                }

                // Weergeven
                sprintf(levensduur_txtStandaardLevensduurText, "%.2f", levensduur_Berekend);
            }
            else
            {
                sprintf(levensduur_txtStandaardLevensduurText, "%d", 0);
            }
        }

        // Berekenen aangepaste levensduur
        if (levensduur_ddbBetrouwbaarheidActive || levensduur_vbFatigueLoadLimitEditMode || levensduur_vbProperheidEditMode || levensduur_vbSmeringsverhoudingEditMode || oud_levensduur_Berekend != levensduur_Berekend)
        {
            // Indien er een wijziging is gebeurd, is de standaard levensduur waarschijnlijk ook veranderd.
            // Nieuwe levensduur opslaan, zodat men kan controleren wanneer die verandert
            oud_levensduur_Berekend = levensduur_Berekend;
            
            // Bepalen of er iets kan worden berekend
            if(levensduur_Berekend != 0.0 && levensduur_vbFatigueLoadLimitValue != 0.0 && levensduur_vbProperheidValue != 0.0 && levensduur_vbSmeringsverhoudingValue != 0.0)
            {
                // Type lager bepalen
                // ddbLagertypeActieve komt overeen met enum lagertype

                // Betrouwbaarheid bepalen
                double betrouwbaarheidsfactor = 1;
                switch (levensduur_ddbBetrouwbaarheidActive)
                {
                case 0: // 90%
                    betrouwbaarheidsfactor = 1;
                    break;
                case 1: //95%
                    betrouwbaarheidsfactor = 0.64;
                    break;
                case 2: // 99%
                    betrouwbaarheidsfactor = 0.25;
                    break;
                case 3: // 99,95%
                    betrouwbaarheidsfactor = 0.077;
                    break;
                }

                const double aisoFactor = aiso_correctiefactor((enum lagertype)levensduur_ddbLagerTypeActive,levensduur_vbProperheidValue,levensduur_vbFatigueLoadLimitValue,levensduur_vbEquivalenteBelastingValue,levensduur_vbSmeringsverhoudingValue);
                const double aangepasteLevensduur = levensduur_Berekend * aisoFactor * betrouwbaarheidsfactor;
                
                sprintf(levensduur_txtAangepasteLevensduurText,"%0.2f",aangepasteLevensduur);
            }
            else
            {
                sprintf(levensduur_txtAangepasteLevensduurText, "%d", 0);
            }
        }

        // Berekenen gemiddelde diameter
        if (smering_vbBuitendiameterEditMode || smering_vbBinnendiameterEditMode)
        {
            if (smering_vbBuitendiameterValue > 0 && smering_vbBinnendiameterValue > 0)
            {
                gemiddeldeDiameter_Berekend = (smering_vbBinnendiameterValue + smering_vbBuitendiameterValue) / 2.0;
            }
            else
            {
                gemiddeldeDiameter_Berekend = 0;
            }

            sprintf(smering_txtGemiddeldeDiameterText, "%.2f", gemiddeldeDiameter_Berekend);
        }
        
        // Aanpassen zodat het functie gebruikt
        // Nodige viscositeit berekenen
        if (gemiddeldeDiameter_Berekend != oud_gemiddeldeDiameter_Berekend || smering_vbToerentalSmeringEditMode)
        {
            oud_gemiddeldeDiameter_Berekend = gemiddeldeDiameter_Berekend;

            if(gemiddeldeDiameter_Berekend > 0 && smering_vbToerentalSmeringValue > 0)
            {
                nodigeViscositeit_Berekend = NodigeViscositeit(gemiddeldeDiameter_Berekend, smering_vbToerentalSmeringValue);
            }
            else
            {
                nodigeViscositeit_Berekend = 0;
            }
            sprintf(smering_txtNodigeViscositeitText,"%.2f", nodigeViscositeit_Berekend);
        }
        
        // Effectieve viscositeit berekenen
        if(smering_vbGebruikteViscositeitEditMode || smering_vbWerkingsTemperatuurEditMode)
        {
            if(smering_vbGebruikteViscositeitValue > 0 && smering_vbWerkingsTemperatuurValue > 0)
            {
                effectieveViscositeit_Berekend = viscositeitOpTemperatuur(smering_vbGebruikteViscositeitValue, smering_vbWerkingsTemperatuurValue);
            }
            else
            {
                effectieveViscositeit_Berekend = 0;
            }

            sprintf(smering_txtEffectieveViscositeitText, "%.2f", effectieveViscositeit_Berekend);
        }

        // Smeringsverhouding
        if(nodigeViscositeit_Berekend != oud_nodigeViscositeit_Berekend || effectieveViscositeit_Berekend != oud_effectieveViscositeit_Berekend)
        {
            oud_nodigeViscositeit_Berekend = nodigeViscositeit_Berekend;
            oud_effectieveViscositeit_Berekend = effectieveViscositeit_Berekend;

            if(nodigeViscositeit_Berekend > 0 && effectieveViscositeit_Berekend > 0)
            {
                kWaarde = effectieveViscositeit_Berekend/nodigeViscositeit_Berekend;
            }
            else
            {
                kWaarde = 0;
            }

            sprintf(smering_txtBerekendeSmeringsverhoudingText, "%.2f", kWaarde);
        }

        // Onbalanskracht
        if(centrifugaalbelasting_vbOnbalansmassaEditMode || centrifugaalbelasting_vbStraalEditMode || centrifugaalbelasting_vbToerentalEditMode)
        {
            if(centrifugaalbelasting_vbOnbalansmassaValue != 0.0 && centrifugaalbelasting_vbStraalValue != 0.0 && centrifugaalbelasting_vbToerentalValue != 0.0)
            {
                // F = m*r*omgega^2
                const double omega = 2 * M_PI * (centrifugaalbelasting_vbToerentalValue/60.0);
                onbalanskracht_Berekend = centrifugaalbelasting_vbOnbalansmassaValue * centrifugaalbelasting_vbStraalValue * pow(omega, 2);
            }
            else
            {
                onbalanskracht_Berekend = 0.0;
            }

            char tekst[33] = "";

            sprintf(tekst, "%.2f", onbalanskracht_Berekend);
            strcpy(centrifugaalbelasting_txtOnbalanskrachtBerekendText, tekst);
            strcpy(centrifugaalbelasting_vbOnbalanskrachtGekozenText, tekst);
            centrifugaalbelasting_vbOnbalanskrachtGekozenValue = onbalanskracht_Berekend;
            // Nog toevoegen dat het volgend vak ook wordt ingevuld.
            
        }

        // Samengestelde belasting
        if(equivalenteSamengesteldeCentrifugaalBelasting != oud_equivalenteSamengesteldeCentrifugaalBelasting)
        {
            oud_equivalenteSamengesteldeCentrifugaalBelasting = equivalenteSamengesteldeCentrifugaalBelasting;

            sprintf(centrifugaalbelasting_txtEquivalenteBelastingText, "%.2f", equivalenteSamengesteldeCentrifugaalBelasting);
        }
        
        // Lagergegevenstabel
        if(!lagergegevens_ddbGevondenLagerEditMode && lagergegevens_ddbGevondenLagerFlankbit) // Negatieve flankdetectie
        {
            // De mogelijk oude tabel verwijderen en een nieuwe tabel aanmaken en hierin de waarden invullen
            if(lagergegevens_Tabelvakken != NULL)
            {
                textvakkenVerwijderen(lagergegevens_Tabelvakken, lagergegevens_Rijen, lagergegevens_kolommen);
            }

            // Nieuwe textvakken aanmaken
            // Bepalen welk lager is gekozen.
            char* gekozenLager = lagergegevensGevondenLagers.lagers[lagergegevens_ddbGevondenLagerActive];
            // Lagergegevens ophalen
            // Oude lagerinformatie verwijderen
            free_lagerinformatie(&lagergegevensGekozenLager);
            // Nieuwe lagerinformatie zoeken
            lagergegevensGekozenLager = ExactZoeken(gekozenLager);


            // Tabel aanmaken
            lagergegevens_Tabelvakken = textvakkenMaken(lagergegevensGekozenLager.aantalGegevens, 2);

            strcpy(lagergegevens_lblTypeLager, LagersoortNaarString(lagergegevensGekozenLager.lagersoort));

            for(size_t i = 0; i < lagergegevensGekozenLager.aantalGegevens; i++)
            {
                strcpy(lagergegevens_Tabelvakken[i][0].Text, lagergegevensGekozenLager.kolomtitels[i]);
                strcpy(lagergegevens_Tabelvakken[i][1].Text, lagergegevensGekozenLager.lagergegevens[i]);
            }
            lagergegevens_Rijen = lagergegevensGekozenLager.aantalGegevens;
        }

        // Flankdetectie
        lagergegevens_ddbGevondenLagerFlankbit = lagergegevens_ddbGevondenLagerEditMode;

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            // Anders blijft het oude staan
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

            //GuiLoadStyle("../src/jungle.rgs");
            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            // Draw controls
            if (levensduur_ddbLagerTypeEditMode || levensduur_ddbBetrouwbaarheidEditMode) GuiLock(); // ?

            // Levensduurberekening
            //-------------------------------------------------------------------------------------
            // Labels
            GuiGroupBox((Rectangle){ 10, 15, 325, 325 }, "Levensduur");
            GuiLabel((Rectangle){ 25, 25, 126, 25 }, "Type lager:");
            GuiLabel((Rectangle){ 25, 50, 126, 25 }, "Dynamisch draaggetal (C):");
            GuiLabel((Rectangle){ 25, 75, 126, 25 }, "Equivalente belasting (P):");
            GuiLabel((Rectangle){ 25, 100, 126, 25 }, "Toerental (n/min):");
            GuiLabel((Rectangle){ 25, 125, 126, 25 }, "Tijdsbasis:");
            GuiLabel((Rectangle){ 25, 150, 115, 25 }, "Standaard Levensduur (L10):");
            GuiLabel((Rectangle){ 25, 200, 126, 25 }, "Betrouwbaarheid (a1):");
            GuiLabel((Rectangle){ 25, 225, 120, 25 }, "Fatigue Load limit (Cu):");
            GuiLabel((Rectangle){ 25, 250, 126, 25 }, "Properheid (ec):");
            GuiLabel((Rectangle){ 25, 275, 126, 25 }, "Smeringsverhouding (k):");
            GuiLabel((Rectangle){ 25, 300, 140, 25 }, "Aangepaste Levensduur:");
            // Valuebox_Double
            if (GuiValueBox_double((Rectangle){ 175, 50, 150, 25 }, levensduur_vbDynamischDraaggetalText, 32, levensduur_vbDynamischDraaggetalEditMode, &levensduur_vbDynamischDraaggetalValue, levensduur_oud_vbDynamischDraaggetalText, 0.0, 10000.0)) levensduur_vbDynamischDraaggetalEditMode = !levensduur_vbDynamischDraaggetalEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 75, 150, 25 }, levensduur_vbEquivalenteBelastingText, 32, levensduur_vbEquivalenteBelastingEditMode, &levensduur_vbEquivalenteBelastingValue, levensduur_oud_vbEquivalenteBelastingText, 0.0, 10000.0))  levensduur_vbEquivalenteBelastingEditMode = !levensduur_vbEquivalenteBelastingEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 100, 150, 25 }, levensduur_vbToerentalText, 32, levensduur_vbToerentalEditMode, &levensduur_vbToerentalValue, levensduur_oud_vbToerentalText, 0.0, 10000.0)) levensduur_vbToerentalEditMode = !levensduur_vbToerentalEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 225, 150, 25 }, levensduur_vbFatigueLoadLimitText, 32, levensduur_vbFatigueLoadLimitEditMode, &levensduur_vbFatigueLoadLimitValue, levensduur_oud_vbFatigueLoadLimitText, 0.0, 10000.0)) levensduur_vbFatigueLoadLimitEditMode = !levensduur_vbFatigueLoadLimitEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 250, 150, 25 }, levensduur_vbProperheidText, 32, levensduur_vbProperheidEditMode, &levensduur_vbProperheidValue, levensduur_oud_vbProperheidText,0.0,1.0)) levensduur_vbProperheidEditMode = !levensduur_vbProperheidEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 275, 150, 25 }, levensduur_vbSmeringsverhoudingText, 32, levensduur_vbSmeringsverhoudingEditMode, &levensduur_vbSmeringsverhoudingValue, levensduur_oud_vbSmeringsverhoudingText, 0, 10.0)) levensduur_vbSmeringsverhoudingEditMode = !levensduur_vbSmeringsverhoudingEditMode;
            // Textbox
            GuiDisable();
            if (GuiTextBox((Rectangle){ 175, 150, 150, 25 }, levensduur_txtStandaardLevensduurText, 32, levensduur_txtStandaardLevensduurEditMode)) levensduur_txtStandaardLevensduurEditMode = !levensduur_txtStandaardLevensduurEditMode;
            if (GuiTextBox((Rectangle){ 175, 300, 150, 25 }, levensduur_txtAangepasteLevensduurText, 32, levensduur_txtAangepasteLevensduurEditMode)) levensduur_txtAangepasteLevensduurEditMode = !levensduur_txtAangepasteLevensduurEditMode;
            GuiEnable();
            // Lijn
            GuiLine((Rectangle){ 10, 175, 325, 25 }, NULL);
            // Combobox
            levensduur_cmbTijdsbasisActive = GuiComboBox((Rectangle){ 175, 125, 150, 25 }, "Omwentelingen; Uren; Dagen; Jaren", levensduur_cmbTijdsbasisActive);
            
            
            // Smering
            //--------------------------------------------------------------------------------
            GuiGroupBox((Rectangle){ 350, 15, 280, 275 }, "Smering");
            // Label
            GuiLabel((Rectangle){ 365, 25, 126, 25 }, "Buitendiameter (D):");
            GuiLabel((Rectangle){ 365, 50, 126, 25 }, "Binnendiameter (d):");
            GuiLabel((Rectangle){ 365, 75, 126, 25 }, "Gemiddelde diameter (dm):");
            GuiLabel((Rectangle){ 365, 100, 126, 25 }, "Toerental (n/min):");
            GuiLabel((Rectangle){ 365, 125, 126, 25 }, "Nodige viscositeit (mm2/s):");
            GuiLabel((Rectangle){ 365, 175, 126, 25 }, "Gebruikte viscositeit (ISO VG):");
            GuiLabel((Rectangle){ 365, 200, 126, 25 }, "Werkingstemperatuur (°C):");
            GuiLabel((Rectangle){ 365, 225, 126, 25 }, "Effectieve viscositeit (mm²/s):");
            GuiLabel((Rectangle){ 365, 250, 126, 25 }, "Smeringsverhouding (k):");
            // ValueBox_double
            if (GuiValueBox_double((Rectangle){ 540, 25, 75, 25 }, smering_vbBuitendiameterText, 32, smering_vbBuitendiameterEditMode, &smering_vbBuitendiameterValue, smering_oud_vbBuitendiameterText, 0, 1000.0)) smering_vbBuitendiameterEditMode = !smering_vbBuitendiameterEditMode;
            if (GuiValueBox_double((Rectangle){ 540, 50, 75, 25 }, smering_vbBinnendiameterText, 32, smering_vbBinnendiameterEditMode, &smering_vbBinnendiameterValue, smering_oud_vbBinnendiameterText, 0, 1000.0)) smering_vbBinnendiameterEditMode = !smering_vbBinnendiameterEditMode;
            if (GuiValueBox_double((Rectangle){ 540, 100, 75, 25 }, smering_vbToerentalSmerintText, 32, smering_vbToerentalSmeringEditMode, &smering_vbToerentalSmeringValue, smering_oud_vbToerentalSmerintText, 0, 10000.0)) smering_vbToerentalSmeringEditMode = !smering_vbToerentalSmeringEditMode;
            if (GuiValueBox_double((Rectangle){ 540, 175, 75, 25 }, smering_vbGebruikteViscositeitText, 32, smering_vbGebruikteViscositeitEditMode, &smering_vbGebruikteViscositeitValue, smering_oud_vbGebruikteViscositeitText, 0, 2000.0)) smering_vbGebruikteViscositeitEditMode = !smering_vbGebruikteViscositeitEditMode;
            if (GuiValueBox_double((Rectangle){ 540, 200, 75, 25 }, smering_vbWerkingsTemperatuurText, 32, smering_vbWerkingsTemperatuurEditMode, &smering_vbWerkingsTemperatuurValue, smering_oud_vbWerkingsTemperatuurText, 0, 150.0)) smering_vbWerkingsTemperatuurEditMode = !smering_vbWerkingsTemperatuurEditMode;
            // TextBox
            GuiDisable();
            if (GuiTextBox((Rectangle){ 540, 75, 75, 25 }, smering_txtGemiddeldeDiameterText, 32, smering_txtGemiddeldeDiameterEditMode)) smering_txtGemiddeldeDiameterEditMode = !smering_txtGemiddeldeDiameterEditMode;
            if (GuiTextBox((Rectangle){ 540, 125, 75, 25 }, smering_txtNodigeViscositeitText, 32, smering_txtNodigeViscositeitEditMode)) smering_txtNodigeViscositeitEditMode = !smering_txtNodigeViscositeitEditMode;
            if (GuiTextBox((Rectangle){ 540, 225, 75, 25 }, smering_txtEffectieveViscositeitText, 32, smering_txtEffectieveViscositeitEditMode)) smering_txtEffectieveViscositeitEditMode = !smering_txtEffectieveViscositeitEditMode;
            if (GuiTextBox((Rectangle){ 540, 250, 75, 25 }, smering_txtBerekendeSmeringsverhoudingText, 32, smering_txtBerekendeSmeringsverhoudingEditMode)) smering_txtBerekendeSmeringsverhoudingEditMode = !smering_txtBerekendeSmeringsverhoudingEditMode;
            GuiEnable();
            // lijn
            GuiLine((Rectangle){ 350, 150, 280, 25 }, NULL);
            
            // Samengestelde centrifugaalbelasting
            // ------------------------------------------------------------------------------------------------------------------
            GuiGroupBox((Rectangle){645, 15, 280, 275}, "Samengestelde centrifugaalbelasting");
            // Togglegroup
            centrifugaalbelasting_tgLagertypeActive = GuiToggleGroup((Rectangle){ 660, 23, 125, 25}, "Radiaal Kogellager; Radiaal Rollager", centrifugaalbelasting_tgLagertypeActive);
            // Label
            //GuiLabel((Rectangle){ 660, 25, 126, 25 }, "Type lager:");
            GuiLabel((Rectangle){ 660, 50, 126, 25 }, "Onbalansmassa (kg):");
            GuiLabel((Rectangle){ 660, 75, 126, 25 }, "Straal (m):");
            GuiLabel((Rectangle){ 660, 100, 126, 25 }, "Toerental (n/min):");
            GuiLabel((Rectangle){ 660, 125, 126, 25 }, "Onbalanskracht (N):");
            GuiLabel((Rectangle){ 660, 175, 126, 25 }, "Onbalanskracht (N):");
            GuiLabel((Rectangle){ 660, 200, 126, 25 }, "Statische kracht (N):");
            GuiLabel((Rectangle){ 660, 225, 126, 25 }, "Equivalente belasting (N):");
            // ValueBox_double
            if (GuiValueBox_double((Rectangle){ 835, 50, 75, 25 }, centrifugaalbelasting_vbOnbalansmassaText, 32, centrifugaalbelasting_vbOnbalansmassaEditMode, &centrifugaalbelasting_vbOnbalansmassaValue, centrifugaalbelasting_oud_vbOnbalansmassaText, 0.0, 10000.0)) centrifugaalbelasting_vbOnbalansmassaEditMode = !centrifugaalbelasting_vbOnbalansmassaEditMode;
            if (GuiValueBox_double((Rectangle){ 835, 75, 75, 25 }, centrifugaalbelasting_vbStraalText, 32, centrifugaalbelasting_vbStraalEditMode, &centrifugaalbelasting_vbStraalValue, centrifugaalbelasting_oud_vbStraalText, 0.0, 10.0)) centrifugaalbelasting_vbStraalEditMode = !centrifugaalbelasting_vbStraalEditMode;
            if (GuiValueBox_double((Rectangle){ 835, 100, 75, 25 }, centrifugaalbelasting_vbToerentalText, 32, centrifugaalbelasting_vbToerentalEditMode, &centrifugaalbelasting_vbToerentalValue, centrifugaalbelasting_oud_vbToerentalText, 0.0, 10000.0)) centrifugaalbelasting_vbToerentalEditMode = !centrifugaalbelasting_vbToerentalEditMode;
            if (GuiValueBox_double((Rectangle){ 835, 175, 75, 25 }, centrifugaalbelasting_vbOnbalanskrachtGekozenText, 32, centrifugaalbelasting_vbOnbalanskrachtGekozenEditMode, &centrifugaalbelasting_vbOnbalanskrachtGekozenValue, centrifugaalbelasting_oud_vbOnbalanskrachtGekozenText, 0.0, 10000.0)) centrifugaalbelasting_vbOnbalanskrachtGekozenEditMode = !centrifugaalbelasting_vbOnbalanskrachtGekozenEditMode;
            if (GuiValueBox_double((Rectangle){ 835, 200, 75, 25 }, centrifugaalbelasting_vbStatischeKrachtText, 32, centrifugaalbelasting_vbStatischeKrachtEditMode, &centrifugaalbelasting_vbStatischeKrachtValue, centrifugaalbelasting_oud_vbStatischeKrachtText, 0.0, 10000.0)) centrifugaalbelasting_vbStatischeKrachtEditMode = !centrifugaalbelasting_vbStatischeKrachtEditMode;
            // Textbox
            GuiDisable();
            if (GuiTextBox((Rectangle){ 835, 125, 75, 25 }, centrifugaalbelasting_txtOnbalanskrachtBerekendText, 32, centrifugaalbelasting_txtOnbalanskrachtBerekendEditMode)) centrifugaalbelasting_txtOnbalanskrachtBerekendEditMode = !centrifugaalbelasting_txtOnbalanskrachtBerekendEditMode;
            if (GuiTextBox((Rectangle){ 835, 225, 75, 25 }, centrifugaalbelasting_txtEquivalenteBelastingText, 32, centrifugaalbelasting_txtEquivalenteBelastingEditMode)) centrifugaalbelasting_txtEquivalenteBelastingEditMode = !centrifugaalbelasting_txtEquivalenteBelastingEditMode;
            GuiEnable();
            // Lijn
            GuiLine((Rectangle){ 645, 150, 280, 25 }, NULL);
            // Knop
            if (GuiButton((Rectangle){660, 252, 250, 25},"Berekenen")) centrifugaalbelasting_btnBerekenen(centrifugaalbelasting_vbStatischeKrachtValue, centrifugaalbelasting_vbOnbalanskrachtGekozenValue, centrifugaalbelasting_tgLagertypeActive, &equivalenteSamengesteldeCentrifugaalBelasting);


            // Veranderlijke belasting
            // -----------------------------------------------------------------------------------------------------------------
            GuiGroupBox((Rectangle){10,360, 325, 425}, "Variabele belasting");
            // Label
            GuiLabel((Rectangle){ 25, 370, 126, 25 }, "Type lager:");
            GuiLabel((Rectangle){ 25, 395, 126, 25 }, "Aiso-factor:");
            GuiLabel((Rectangle){ 25, 420, 126, 25 }, "Viscositeit (ISO VG):");
            GuiLabel((Rectangle){ 25, 445, 126, 25 }, "Temperatuur (°C):");
            GuiLabel((Rectangle){ 25, 470, 126, 25 }, "Fatigue load limit (N):");
            GuiLabel((Rectangle){ 25, 495, 126, 25 }, "Properheid (ec):");
            GuiLabel((Rectangle){ 25, 520, 126, 25 }, "Gemiddelde diameter (mm):");
            GuiLabel((Rectangle){ 25, 570, 126, 25 }, "Tijdstip (s):");
            GuiLabel((Rectangle){ 25, 595, 126, 25 }, "Toerental (n/min):");
            GuiLabel((Rectangle){ 25, 620, 126, 25 }, "Belasting (N):");
            GuiLabel((Rectangle){ 25, 695, 126, 25 }, "Equivalente belasting (N):");
            GuiLabel((Rectangle){ 25, 720, 126, 25 }, "Gemiddeld toerental (n/min):");
            // Combobox
            veranderlijkeBelasting_cmbLagerTypeActive = GuiComboBox((Rectangle){ 175, 370, 150, 25 }, "Radiaal Kogellager; Radiaal Rollager; Axiaal Kogellager; Axiaal Rollager", veranderlijkeBelasting_cmbLagerTypeActive);
            // Checkbox
            veranderlijkeBelasting_cbAisoFactor = GuiCheckBox((Rectangle){175, 395 + 7, 12, 12}, "Berekenen", veranderlijkeBelasting_cbAisoFactor);
            // Valuebox
            if(!veranderlijkeBelasting_cbAisoFactor)
            {
                GuiDisable();
            }
            if (GuiValueBox_double((Rectangle){ 175, 420, 150, 25 }, veranderlijkeBelasting_vbViscositeitText, 32, veranderlijkeBelasting_vbViscositeitEditMode, &veranderlijkeBelasting_vbViscositeitValue, veranderlijkeBelasting_oud_vbViscositeitText, 0, 2000.0)) veranderlijkeBelasting_vbViscositeitEditMode = !veranderlijkeBelasting_vbViscositeitEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 445, 150, 25 }, veranderlijkeBelasting_vbTemperatuurText, 32, veranderlijkeBelasting_vbTemperatuurEditMode, &veranderlijkeBelasting_vbTemperatuurValue, veranderlijkeBelasting_oud_vbTemperatuurText, 0, 120.0)) veranderlijkeBelasting_vbTemperatuurEditMode = !veranderlijkeBelasting_vbTemperatuurEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 470, 150, 25 }, veranderlijkeBelasting_vbFatigueText, 32, veranderlijkeBelasting_vbFatigueEditMode, &veranderlijkeBelasting_vbFatigueValue, veranderlijkeBelasting_oud_vbFatigueText, 0, 100000.0)) veranderlijkeBelasting_vbFatigueEditMode = !veranderlijkeBelasting_vbFatigueEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 495, 150, 25 }, veranderlijkeBelasting_vbProperheidText, 32, veranderlijkeBelasting_vbProperheidEditMode, &veranderlijkeBelasting_vbProperheidValue, veranderlijkeBelasting_oud_vbProperheidText, 0, 1.0)) veranderlijkeBelasting_vbProperheidEditMode = !veranderlijkeBelasting_vbProperheidEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 520, 150, 25 }, veranderlijkeBelasting_vbGemiddeldeDiameterText, 32, veranderlijkeBelasting_vbGemiddeldeDiameterEditMode, &veranderlijkeBelasting_vbGemiddeldeDiameterValue, veranderlijkeBelasting_oud_vbGemiddeldeDiameterText, 0, 1000.0)) veranderlijkeBelasting_vbGemiddeldeDiameterEditMode = !veranderlijkeBelasting_vbGemiddeldeDiameterEditMode;
            GuiEnable();

            if (GuiValueBox_double((Rectangle){ 175, 570, 150, 25 }, veranderlijkeBelasting_vbTijdstipText, 32, veranderlijkeBelasting_vbTijdstipEditMode, &veranderlijkeBelasting_vbTijdstipValue, veranderlijkeBelasting_oud_vbTijdstipText, 0, 100000.0)) veranderlijkeBelasting_vbTijdstipEditMode = !veranderlijkeBelasting_vbTijdstipEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 595, 150, 25 }, veranderlijkeBelasting_vbToerentalText, 32, veranderlijkeBelasting_vbToerentalEditMode, &veranderlijkeBelasting_vbToerentalValue, veranderlijkeBelasting_oud_vbToerentalText, 0, 100000.0)) veranderlijkeBelasting_vbToerentalEditMode = !veranderlijkeBelasting_vbToerentalEditMode;
            if (GuiValueBox_double((Rectangle){ 175, 620, 150, 25 }, veranderlijkeBelasting_vbBelastingText, 32, veranderlijkeBelasting_vbBelastingEditMode, &veranderlijkeBelasting_vbBelastingValue, veranderlijkeBelasting_oud_vbBelastingText, 0, 100000.0)) veranderlijkeBelasting_vbBelastingEditMode = !veranderlijkeBelasting_vbBelastingEditMode;
            // Textbox
            GuiDisable();
            if (GuiTextBox((Rectangle){ 175, 695, 150, 25 }, veranderlijkeBelasting_txtEquivalenteBelastingText, 32, veranderlijkeBelasting_txtEquivalenteBelastingEditMode)) veranderlijkeBelasting_txtEquivalenteBelastingEditMode = !veranderlijkeBelasting_txtEquivalenteBelastingEditMode;
            if (GuiTextBox((Rectangle){ 175, 720, 150, 25 }, veranderlijkeBelasting_txtGemiddeldToerentalText, 32, veranderlijkeBelasting_txtGemiddeldToerentalEditMode)) veranderlijkeBelasting_txtGemiddeldToerentalEditMode = !veranderlijkeBelasting_txtGemiddeldToerentalEditMode;
            GuiEnable();
            // Knop
            if (GuiButton((Rectangle){ 25, 647, 148, 25}, "Toevoegen")) veranderlijkeBelasting_btnToevoegen(veranderlijkeBelasting_vbTijdstipValue, veranderlijkeBelasting_vbToerentalValue, veranderlijkeBelasting_vbBelastingValue);
            if (GuiButton((Rectangle){ 175, 647, 150, 25}, "Reset")) veranderlijkeBelasting_btnReset();
            if (GuiButton((Rectangle){ 25, 747, 300, 25}, "Bereken")) veranderlijkeBelasting_btnBereken(veranderlijkeBelasting_txtEquivalenteBelastingText, veranderlijkeBelasting_txtGemiddeldToerentalText, veranderlijkeBelasting_cmbLagerTypeActive, veranderlijkeBelasting_cbAisoFactor, veranderlijkeBelasting_vbViscositeitValue, veranderlijkeBelasting_vbTemperatuurValue, veranderlijkeBelasting_vbFatigueValue, veranderlijkeBelasting_vbProperheidValue, veranderlijkeBelasting_vbGemiddeldeDiameterValue);
            // lijn
            GuiLine((Rectangle){ 10, 545, 325, 25}, NULL);
            GuiLine((Rectangle){ 10, 670, 325, 25}, NULL);

            // Tabel
            GuiGroupBox((Rectangle){940, 15, 255, 770}, "Tabel");
            // Label    
            GuiLabel((Rectangle){955, 25,126,25}, "Tijd");
            GuiLabel((Rectangle){1030, 25,126,25}, "Toerental");
            GuiLabel((Rectangle){1105, 25,126,25}, "Belasting");
            GuiTabel(955, 50, veranderlijkeBelasting_Rijen, veranderlijkeBelasting_Kolommen, 75,25, veranderlijkeBelasting_Tabelvakken);
            
            // Grafiek
            //--------------------------------------------------------------------------------
            GuiGroupBox((Rectangle){350, 310, 575, 475}, "Grafiek");
            DrawRectangleLines(365, 320, 545, 455, GetColor(0x90abb5ff));
            GuiGrid((Rectangle) {365, 320, 545, 455}, 45.4f, 5.f);
            grafiekTekenen();


            // Lagergegevens
            //---------------------------------------------------------------------------------
            GuiGroupBox((Rectangle){1210, 15, 280, 470}, "Lagergegevens");
            // Label
            GuiLabel((Rectangle){ 1225, 25, 126, 25 }, "Lagercode:");
            GuiLabel((Rectangle){ 1225, 100, 126, 25 }, "Type lager:");
            GuiLabel((Rectangle){ 1350, 100, 126, 25 }, lagergegevens_lblTypeLager);
            // Knop
            if (GuiButton((Rectangle){ 1225, 52, 123, 25}, "Zoeken")) 
            {
                lagergegevens_btnZoeken(lagergegevens_txtLagercodeText, lagergegevens_ddbGevondenLagerText, &lagergegevensGevondenLagers);
                lagergegevens_ddbGevondenLagerActive = 0;
            }
            // Textbox
            if (GuiTextBox((Rectangle){ 1350, 25, 125, 25 }, lagergegevens_txtLagercodeText, 32, lagergegevens_txtLagercodeEditMode)) lagergegevens_txtLagercodeEditMode = !lagergegevens_txtLagercodeEditMode;
            // Lijn
            GuiLine((Rectangle){1210, 75, 280, 25}, NULL);
            // Tabel
            GuiTabel(1225, 125, lagergegevens_Rijen, lagergegevens_kolommen, 125, 25, lagergegevens_Tabelvakken);
            

            // Equivalente belasting
            //---------------------------------------------------------------------
            GuiGroupBox((Rectangle){1210, 500, 280, 124}, "Equivalente belasting");
            // Label
            GuiLabel((Rectangle){1225, 510, 125, 25}, "Radiaalkracht (N):");
            GuiLabel((Rectangle){1225, 535, 125, 25}, "Axiaalkracht (N):");
            GuiLabel((Rectangle){1225, 589, 125, 25}, "Equivalente belasting (N):");
            // Double Valuebox
            if (GuiValueBox_double((Rectangle){ 1375, 510, 100, 25 }, equivalenteBelasting_vbRadialeKrachtText, 32, equivalenteBelasting_vbRadialeKrachtEditMode, &equivalenteBelasting_vbRadialeKrachtValue, equivalenteBelasting_oud_vbRadialeKrachtText,0.0, 10000000.0)) equivalenteBelasting_vbRadialeKrachtEditMode = !equivalenteBelasting_vbRadialeKrachtEditMode;
            if (GuiValueBox_double((Rectangle){ 1375, 535, 100, 25 }, equivalenteBelasting_vbAxialeKrachtText, 32, equivalenteBelasting_vbAxialeKrachtEditMode, &equivalenteBelasting_vbAxialeKrachtValue, equivalenteBelasting_oud_vbAxialeKrachtText,0.0, 10000000.0)) equivalenteBelasting_vbAxialeKrachtEditMode = !equivalenteBelasting_vbAxialeKrachtEditMode;
            // Textbox
            GuiTextBox((Rectangle){1375, 589, 100, 25}, equivalenteBelasting_txtEquivalenteBelastingText, 32, false);
            // knop
            if(lagergegevensGekozenLager.aantalGegevens == 0)
            {
                GuiDisable();
            }
            if (GuiButton((Rectangle){ 1225, 562, 250, 25}, "Bereken")) equivalenteBelasting_btnBerekenen(lagergegevensGekozenLager, equivalenteBelasting_vbRadialeKrachtValue, equivalenteBelasting_vbAxialeKrachtValue, equivalenteBelasting_txtEquivalenteBelastingText);
            GuiEnable();
            
            // Dropdownboxen moeten op het einde komen
            GuiUnlock(); // ?
            if (GuiDropdownBox((Rectangle){ 175, 200, 150, 25 }, "90%;95%;99%;99,95%", &levensduur_ddbBetrouwbaarheidActive, levensduur_ddbBetrouwbaarheidEditMode)) levensduur_ddbBetrouwbaarheidEditMode = !levensduur_ddbBetrouwbaarheidEditMode;
            if (GuiDropdownBox((Rectangle){ 175, 25, 150, 25 }, "Radiaal Kogellager; Radiaal Rollager; Axiaal Kogellager; Axiaal Rollager", &levensduur_ddbLagerTypeActive, levensduur_ddbLagerTypeEditMode)) levensduur_ddbLagerTypeEditMode = !levensduur_ddbLagerTypeEditMode;
            if (GuiDropdownBox((Rectangle){ 1350, 52, 125, 25}, lagergegevens_ddbGevondenLagerText, &lagergegevens_ddbGevondenLagerActive, lagergegevens_ddbGevondenLagerEditMode)) lagergegevens_ddbGevondenLagerEditMode = !lagergegevens_ddbGevondenLagerEditMode;
            

            GuiUnlock();
            //----------------------------------------------------------------------------------
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------

/**
 * @brief Knop om samengestelde belasting te berekenen
 * 
 * @param statisch De statische kracht in Newton
 * @param dynamisch De dynamische kracht of centrifugaalkracht in Newton
 * @param lagertype 0 = kogel; 1= rol
 * @param resultaat Plaats waar het resultaat van de berekening moet worden opgeslagen
 */
static void centrifugaalbelasting_btnBerekenen(double statisch, double dynamisch, int lagertype, double* resultaat)
{
    double equivalenteBelasting = 0;
    
    if(statisch > 0 && dynamisch >  0)
    {
        double exponent = 3;
        if(lagertype == 1) // rollager
        {
            exponent = 10.0/3.0;
        }

        equivalenteBelasting = samengesteldeBelasting(statisch, dynamisch, exponent);

    }

    *resultaat = equivalenteBelasting;
}

/**
 * @brief Misschien nog toevoegen om vakken op 0 te zetten en dat het nieuwe tijdstip groter moet zijn dan het vorige
 * Gebruikt global veranderlijkeBelasting_Tabelvakken
 * @param tijdstip Tijdstip dat moet worden toegevoegd
 * @param toerental Toerental dat moet worden toegevoegd
 * @param belasting Belasting die moet worden toegevoegd
 */
static void veranderlijkeBelasting_btnToevoegen(double tijdstip, double toerental, double belasting)
{
    // Indien men een rij met extra gegevens wilt invullen, moet 
    if(veranderlijkeBelasting_Rijen == 0 || (veranderlijkeBelasting_IngevuldeGegevens[veranderlijkeBelasting_Rijen -1].tijdstip < tijdstip))
    {
        // Nieuwe rij toevoegen
        veranderlijkeBelasting_Tabelvakken = TekstvakRijToevoegen(veranderlijkeBelasting_Tabelvakken, veranderlijkeBelasting_Rijen, veranderlijkeBelasting_Kolommen);
        
        // Gegevens in rij plaatsen
        sprintf(veranderlijkeBelasting_Tabelvakken[veranderlijkeBelasting_Rijen][0].Text, "%.0f s", tijdstip);
        sprintf(veranderlijkeBelasting_Tabelvakken[veranderlijkeBelasting_Rijen][1].Text, "%.0f n/min", toerental);
        sprintf(veranderlijkeBelasting_Tabelvakken[veranderlijkeBelasting_Rijen][2].Text, "%.0f N", belasting);
        
        // Gegevens opslaan voor intern gebruik
        veranderlijkeBelasting_IngevuldeGegevens[veranderlijkeBelasting_Rijen].tijdstip = tijdstip;
        veranderlijkeBelasting_IngevuldeGegevens[veranderlijkeBelasting_Rijen].toerental = toerental;
        veranderlijkeBelasting_IngevuldeGegevens[veranderlijkeBelasting_Rijen].belasting = belasting;
        
        
        // Teller vermeerderen. Op het einde omdat in een array alles 1 kleiner is
        veranderlijkeBelasting_Rijen++;
    }
}

/**
 * @brief Alle opgeslagen rijen verwijderen en aantal rijen terug op 0 zetten
 * 
 */
static void veranderlijkeBelasting_btnReset(void)
{
    textvakkenVerwijderen(veranderlijkeBelasting_Tabelvakken, veranderlijkeBelasting_Rijen, veranderlijkeBelasting_Kolommen);
    //veranderlijkeBelasting_Tabelvakken = NULL; // Zou het probleem hebben kunnen oplossen. NULL mag men zoveel vrijmaken als men wilt, maar adres 1 en 2... Zijn waarschijnlijk nog veel minder gedefinieerd dan een ongedifinieerd adres vrijmaken.
    
    veranderlijkeBelasting_Rijen = 0;
}

/**
 * @brief Gaat het equivalente toerental berekenen en de equivalente belasting aan de hand van de invevulde gegevens
 * 
 * @param txtEquivalenteBelasting 
 * @param txtGemiddeldToerental 
 * @param gekozenlager // 0 = kogellager, 1= cilinderlager, 2 = kogeltaatslager, 3 = cilindertaatslager// Komt toevallig overeen met lagertype
 * @param aisoberekenen 
 * @param referentieviscositeit 
 * @param temperatuur 
 * @param fatigueloadlimit 
 * @param properheid 
 * @param gemiddeldediameter 
 */
static void veranderlijkeBelasting_btnBereken(char* txtEquivalenteBelasting, char* txtGemiddeldToerental, int gekozenlager, bool aisoberekenen, double referentieviscositeit, double temperatuur, double fatigueloadlimit, double properheid, double gemiddeldediameter)
{
   // Enkel berekenen indien er 2 rijen of meer zijn zijn
    if(veranderlijkeBelasting_Rijen >= 2)
    {
        // Struct gereedmaken
        veranderlijkeBelasting_argumenten argumenten;
        argumenten.lagertype = gekozenlager;
        argumenten.aisoBerekenen = aisoberekenen;
        if(aisoberekenen)
        {
            argumenten.referentieviscositeit = referentieviscositeit;
            argumenten.gemiddeldediameter = gemiddeldediameter;
            argumenten.werkingstemperatuur = temperatuur;
            argumenten.fatigueloadlimit = fatigueloadlimit;
            argumenten.properheid = properheid;
        }

        double gemiddeldToerental;
        double belasting = veranderlijkeBelasting_Tabel(veranderlijkeBelasting_IngevuldeGegevens, veranderlijkeBelasting_Rijen, argumenten, &gemiddeldToerental);

        // Gevonden gegevens weergeven
        sprintf(txtEquivalenteBelasting, "%.2f", belasting);
        sprintf(txtGemiddeldToerental, "%.2f", gemiddeldToerental);
    }
}

static void lagergegevens_btnZoeken(char *zoekterm, char *lijst, gevondenlagers *delagers)
{
    if (zoekterm[0] != 0)
    {
        // Alle lagers zoeken die voldoen aan de zoekterm
        gevondenlagers lagers = LagersZoeken(zoekterm);

        if (lagers.aantal > 0)
        {
            int aantalTekens = 0; // Houdt bij hoeveel tekens er al in zitten
            for (size_t i = 0; i < lagers.aantal; i++)
            {
                // Gegeven kopiëren
                for (size_t j = 0; j < strlen(lagers.lagers[i]); j++)
                {
                    lijst[aantalTekens + j] = lagers.lagers[i][j];
                }

                aantalTekens += strlen(lagers.lagers[i]);
                // ; toevoegen
                lijst[aantalTekens] = ';'; // lengte is gelijk aan de volgende plaats
                aantalTekens++;
            }
            // Laatste ; vervangen door een 0x00
            lijst[aantalTekens - 1] = 0;

            // Lagers opslaan
            // Oude gegevens eerst verwijderen
            free_gevondenlagers(delagers);
            *delagers = lagers;
        }
        else
        {
            // Er zijn geen lagers gevonden
            *lijst = 0;
        }
    }
}

static void equivalenteBelasting_btnBerekenen(lagerinformatie lager, double radiaalkracht, double axiaalkracht, char* resultaat)
{
    double belasting = equivalenteBelasting(lager, radiaalkracht, axiaalkracht);

    sprintf(resultaat, "%.2f", belasting);
} 

/**
 * @brief Tekent de grafiek aan de hand van de ingevulde gegevens
 * 
 */
static void grafiekTekenen(void)
{
    // Grafiek is 300 breed en 300 hoog. dus pakt 290 hoog
    // Enkel tekenen indien er 2 of meer waarden zijn opgeslagen.
    // We gaan ervan uit dat de waarden chronologisch zijn ingevuld
    if(veranderlijkeBelasting_Rijen >= 2)
    {
        // De maximumwaarden zoeken om de schaal juist te zetten
        double maximumTijd = 0;
        double maximumToerental = 0;
        double maximumBelasting = 0;

        for (int i = 0; i < veranderlijkeBelasting_Rijen; i++)
        {
            if(maximumTijd < veranderlijkeBelasting_IngevuldeGegevens[i].tijdstip)
            {
                maximumTijd = veranderlijkeBelasting_IngevuldeGegevens[i].tijdstip;
            }
            if(maximumToerental < veranderlijkeBelasting_IngevuldeGegevens[i].toerental)
            {
                maximumToerental = veranderlijkeBelasting_IngevuldeGegevens[i].toerental;
            }
            if(maximumBelasting < veranderlijkeBelasting_IngevuldeGegevens[i].belasting)
            {
                maximumBelasting = veranderlijkeBelasting_IngevuldeGegevens[i].belasting;
            }
        } // Maximumwaarden zijn bepaald.

        // incrementen bepalen
        const double incrementTijd = 545.0 / maximumTijd;
        const double incrementToerental = 450.0 / maximumToerental;
        const double incrementBelasting = 450.0 / maximumBelasting;
        
        // Coördinaten linker onderhoek 
        const int offsetX = 365;
        const int offsetY = 775;

        // Lijnen tekenen
        // Eerste punten van de grafiek bepalen
        // Worden éénmalig gebruikt om die initiele punten op te maken. Daarna wordt elk punt overschreven
        const int oudTijd = (int)lrint(veranderlijkeBelasting_IngevuldeGegevens[0].tijdstip * incrementTijd);
        const int oudToerental = (int)lrint(veranderlijkeBelasting_IngevuldeGegevens[0].toerental * incrementToerental);
        const int oudBelasting = (int)lrint(veranderlijkeBelasting_IngevuldeGegevens[0].belasting * incrementBelasting);

        Vector2 oudPuntToerental;
        oudPuntToerental.x = offsetX + oudTijd;
        oudPuntToerental.y = offsetY - oudToerental;

        Vector2 oudPuntBelasting;
        oudPuntBelasting.x = offsetX + oudTijd;
        oudPuntBelasting.y = offsetY - oudBelasting;

        for (int i = 1; i < veranderlijkeBelasting_Rijen; i++) // Beginnen bij het 2de punt
        {
            // gegevens bepalen voor nieuwe 
            // Word elke lus opnieuw bepaald
            const int nieuwTijd = (int)lrint(veranderlijkeBelasting_IngevuldeGegevens[i].tijdstip * incrementTijd);
            const int nieuwToerental = (int)lrint(veranderlijkeBelasting_IngevuldeGegevens[i].toerental * incrementToerental);
            const int nieuwBelasting = (int)lrint(veranderlijkeBelasting_IngevuldeGegevens[i].belasting * incrementBelasting);

            // nieuwe Punten bepalen om te tekenen
            Vector2 nieuwPuntToerental;
            nieuwPuntToerental.x = offsetX + nieuwTijd;
            nieuwPuntToerental.y = offsetY - nieuwToerental;

            Vector2 nieuwPuntBelasting;
            nieuwPuntBelasting.x = offsetX + nieuwTijd;
            nieuwPuntBelasting.y = offsetY - nieuwBelasting;

            
            DrawLineEx(oudPuntToerental, nieuwPuntToerental, 2, GREEN);
            DrawLineEx(oudPuntBelasting, nieuwPuntBelasting, 2, RED);

            oudPuntToerental = nieuwPuntToerental;
            oudPuntBelasting = nieuwPuntBelasting;
        }
    }
}

/**
 * @brief Creëert een textvak[rij][kolom] van de opgegeven grootte en breedte en wijst er geheugen voor toe
 * Geeft de pointer van dubbele array terug. Kan direct worden gebruikt
 * @param rij aantal rijen
 * @param kolom aantal kolommen
 * @return struct textvak** 
 */
static struct textvak** textvakkenMaken(int rij, int kolom)   
{
    // Moet hier geen calloc zijn omdat het toch wordt overschreven
    struct textvak** wijzer = (struct textvak**)malloc(sizeof(struct textvak*) * (size_t)rij); // Wijzer naar lijst met wijzers naar de structs van de textvakken

    for (int i = 0; i < rij; i++)
    {
        // Hierdoor is alles eerst leeg
        wijzer[i] = (struct textvak*)calloc((size_t)kolom, sizeof(struct textvak)); // Elke wijzer in de wijzer laten wijzen een lijst met de structs
        // Een struct is dus effectief in het geheugen en gijn wijzer naar de struct in het geheugen
    }
 
    
    //Char* opvullen
    for (int i = 0; i < rij; i++)
    {
        for (int j = 0; j < kolom; j++)
        {
            // calloc zodat alles leeg is
            wijzer[i][j].Text = (char*)calloc(32, sizeof(char)); // Tekstvakken gereedmaken
        }
        
    }

    return wijzer;
}

/**
 * @brief Gaat de opgegeven textvak[rij][kolom] verwijderen en het geheugen vrijmaken.
 * Het aantal rijen en kolommen moet overeenkomen met het ingevulde textvak
 * Niet gerbuiken op reeds meegemaakte textvakken. Hierdoor gaat men bepaalde gegevens twee keer leegmaken, wat met vertraging voor een fout kan zorgen.
 * @param vakken textvak[][] die moeten wordne vrijgemaakt
 * @param rij De hoeveelheid rijen waaruit het textvak bestaat
 * @param kolom De hoeveelheid kolommen
 */
static void textvakkenVerwijderen(struct textvak **vakken, int rij, int kolom)
{
    // Eerst controleren of er geen lege tabel moet worden leeggemaakt
    if (rij != 0 && kolom != 0)
    {
        // Eerst alle teksten verwijderen
        for (int i = 0; i < rij; i++)
        {
            for (int j = 0; j < kolom; j++)
            {
                free(vakken[i][j].Text);
            }
        }

        // Structs zelf verwijderen
        for (int i = 0; i < rij; i++)
        {
            free(vakken[i]);
        }

        // Pointer naar array van pointers naar structs verwijderen
        free(vakken);
    }
}

/**
 * @brief Maakt een nieuwe textvak[rij][kolom] met één rij meer dan de oude, kopieert alle gegeven en verwijdert de oude textvak[][]. Geeft de nieuwe textvak[][] terug
 * Niet gebruiken indien er net een textvak is verwijderd. Hierdoor zou het twee keer hetzelfde leegmaken, wat ongedefiniëerd gedrag is. kan worden vermeden door de leeggemaakte lijst toe te wijzen aan een Null pointer. Dan gebeurt er niets
 * @param oudtextvak Pointer naar het oude textvak
 * @param oudaantalrijen Oud aantal rijen
 * @param oudaantalkolommen Oud aantal kolommen
 * @return struct textvak** 
 */
static struct textvak** TekstvakRijToevoegen(struct textvak** oudtextvak, int oudaantalrijen, int oudaantalkolommen)
{
    // Nieuw Textvak maken
    struct textvak** nieuwTextvak = textvakkenMaken(oudaantalrijen + 1, oudaantalkolommen);

    // oude gegevens kopiëren
    for (int i = 0; i < oudaantalrijen; i++)
    {
        for (int j = 0; j < oudaantalkolommen; j++)
        {
            nieuwTextvak[i][j].EditMode = oudtextvak[i][j].EditMode;
            strcpy(nieuwTextvak[i][j].Text, oudtextvak[i][j].Text);
        } 
    }
    
    // oud textvak vrijmaken
    if(oudaantalrijen != 0)
    {
        textvakkenVerwijderen(oudtextvak, oudaantalrijen, oudaantalkolommen);
    }
    
    return nieuwTextvak;
}

/**
 * @brief Enkel Positieve getallen
 * Ondersteunt enkel lezen, kan niet naar worden geschreven
 * 
 * @param bounds (rectangle) { X-coördinaat, Y-coördinaat, breedte, hoogte}
 * @param text Weer te geven text
 * @param textsize Grootte bijvoorbeeld 32 tekens
 * @param editMode Edit mode
 * @param value Omgezette waarde die is in gevuld
 * @param oldText Oude text
 * @param minValue Minimumwaarde
 * @param maxValue Maximumwaarde
 * @return true 
 * @return false 
 */
static bool GuiValueBox_double(Rectangle bounds, char *text, int textsize, bool editMode, double *value, char *oldText, double minValue, double maxValue)
{
    if (editMode) // Enkel uitvoeren indien control actief is
    {
        // Controleren of het ingevulde getal wel kan worden uitgelezen, sscanf aanvaard ook letters. Het stopt dan enkel met uitlezen
        int geenVreemdeTekens = true;
        double omgezetGetal;
        
        // Zorgen dat er altijd minstens een 0 in staat
        if (text[0] == 0)
        {
            text[0] = '0';
        }

        // Controleren of er geen vreemde tekens zijn ingevuld
        for (int i = 0; i < textsize; i++)
        {
            if (!((text[i] >= '0' && text[i] <= '9') || text[i] == '.' || text[i] == 0))
            {
                geenVreemdeTekens = false;
            }
        }

        if (sscanf(text, "%lf", &omgezetGetal) && geenVreemdeTekens)
        {
            // Getal is goed
            // Controleren of het niet te groot of te klein is
            if(omgezetGetal > maxValue)
            {
                omgezetGetal = maxValue;
                sprintf(text,"%g",omgezetGetal);
            }
            else if (omgezetGetal < minValue)
            {
                omgezetGetal = minValue;
                sprintf(text,"%g",omgezetGetal);
            }
            // Getal opslaan
            *value = omgezetGetal;
            strcpy(oldText, text);
        }
        else
        {
            // Getal is slecht
            strcpy(text, oldText);
        }

        // Controleren of er geen 0 voor een cijfer staat
        // Enkel eerste twee tekens moeten worden gecontroleerd
        if (text[0] == '0' && (text[1] >= '0' && text[1] <= '9'))
        {
            // 0 in het begin weghalen
            for (int i = 0; i < (textsize - 1); i++) // sizeof -1 omdat het laatste teken niet moet worden veranderd
            {
                text[i] = text[i + 1];
            }
        }

        
    }
    
    return (GuiTextBox(bounds, text, textsize, editMode));
}

/**
 * @brief Tekent een tabel op de opgegeven locatie door middel van textvakken met het opgegeven aantal rijen en kolommen en de individuele grootte van elk textvak.
 * Heeft een textvak[][] nodig om de gegevens in op te slaan
 * @param locatieX X-coördinaat
 * @param locatieY Y-coördinaat
 * @param rijen Aantal rijen Moet overeenkomen met textvak[][]
 * @param kolommen Aantal kolommen Moet overeenkomen met textvak[][]
 * @param breedtetextvak Breedte van elk textvak
 * @param hoogtetextvak Hoogte van elk textvak
 * @param vakken textvak[][]
 */
static void GuiTabel(int locatieX, int locatieY, int rijen, int kolommen, int breedtetextvak, int hoogtetextvak, struct textvak** vakken)
{
    for (int i = 0; i < rijen; i++) // Elke rij
    {
        for (int j = 0; j < kolommen; j++) // Elke kolom
        {
            if (GuiTextBox((Rectangle){ j*breedtetextvak + locatieX, i*hoogtetextvak + locatieY, breedtetextvak, hoogtetextvak }, vakken[i][j].Text, 32, vakken[i][j].EditMode)) vakken[i][j].EditMode = !vakken[i][j].EditMode;
        }
    }
}
