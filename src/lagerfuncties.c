#include "lagerfuncties.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <gsl/gsl_integration.h>
#include <csv.h>

#include "lagers/headers/carblager.h"
#include "lagers/headers/cilinderlagers.h"
#include "lagers/headers/dubbelrijighoekcontactkogellagers.h"
#include "lagers/headers/dubbelrijigkogellager.h"
#include "lagers/headers/dubbelrijigtonlagers.h"
#include "lagers/headers/hoekcontactkogellagers.h"
#include "lagers/headers/kegellagers.h"
#include "lagers/headers/kogellagers.h"
#include "lagers/headers/tonlagers.h"
#include "lagers/headers/vierpuntslagers.h"
#include "lagers/headers/zichinstellandekogellagers.h"

// De 3 standard library headers kan men ook wel laeten. Dan is het uitvoerbaar bestand iets kleiner omdat er dan waarschijnlijk minder symbolen worden aangemaakt. Als men achteraf strip uitvoert blijft de grootte wel hetzelfde bij de twee situaties.
#ifdef WINNI
#include <stdlib.h>
#endif

/**
 * @brief Berekend de standaard lagerlevensduur in miljoen omwentelingen
 * 
 * @param dynamischdraaggetal Dynamisch draaggetal van lager in Newton
 * @param equivalentebelasting Equivalente belasting op lager in Newton
 * @param exponent 3 of 3,33
 * @return double 
 */
double standaardLevensduur(double dynamischdraaggetal, double equivalentebelasting, double exponent)
{
    return pow((dynamischdraaggetal/equivalentebelasting),exponent);
}

/**
 * @brief Gaat in functie van het toerental de levensduur omzetten in functie van aantal draaiuren, draaidagen of draaijaren.
 * 
 * @param levensduur Levensduur in miljoen omwentelingen
 * @param toerental Toerental in n/min
 * @param tijd Tijdsbasis: UUR, DAGEN, JAREN
 * @return double 
 */
double levensduurOpTijd(double levensduur, double toerental, enum tijdsbasis tijd)
{
    switch (tijd)
    {
    case UUR:
        return (16666/toerental)*levensduur;
        //break;
    case DAGEN:
        return (16666/toerental)*levensduur/24;
        //break;
    case JAREN:
        return (16666/toerental)*levensduur/24/365.25;
        //break;
    }
    return 0;
}

/**
 * @brief Berekent de Aiso correctiefactor. Gelimiteerd tot maximaal 50
 * 
 * @param lager Type lager: RADIAAL_KOGEL, RADIAAL_CILINDER, AXIAAL_KOGEL, AXIAAL_CILINDER
 * @param ec Properheid 0-1
 * @param Cu Fatigue load limit
 * @param P Equivelante belasting
 * @param k Smeringsverhouding
 * @return double Aiso correctifactor (max 50), 0 Indien buiten bereik
 */
double aiso_correctiefactor(enum lagertype lager, double ec, double Cu, double P, double k)
{
    double factor = 0; // Blijft 0 indien de k kleiner is dan 0.1 omdat er dan geen iso factor kan worden berekend.

    double kintern = k;
    // K mag maximaal 4 zijn voor de berekening
    if (k>4)
    {
        kintern = 4;
    }

    switch (lager)
    {
    case RADIAAL_KOGEL:
        if(kintern >= 1.0)
        {
            factor = 0.1 * pow(1. - pow(2.5671 - (1.9987 / pow(kintern, 0.071739)), 0.83) * pow((ec * Cu) / P, 1. / 3.), -9.3);
        }
        else if (kintern >= 0.4)
        {
            factor = 0.1 * pow(1. - pow(2.5671 - (1.9987 / pow(kintern, 0.19087)), 0.83) * pow((ec * Cu) / P, 1. / 3.), -9.3);
        }
        else if (kintern >= 0.1)
        {
            factor = 0.1 * pow(1. - pow(2.5671 - (2.2649 / pow(kintern, 0.054381)), 0.83) * pow((ec * Cu) / P, 1. / 3.), -9.3);
        }
        break;
    case RADIAAL_CILINDER:
        if(kintern >= 1.0)
        {
            factor = 0.1 * pow(1. - (1.5859 - (1.2348 / pow(kintern, 0.071739))) * pow((ec * Cu) / P, 0.4), -9.185);
        }
        else if (kintern >= 0.4)
        {
            factor = 0.1 * pow(1. - (1.5859 - (1.2348 / pow(kintern, 0.19087))) * pow((ec * Cu) / P, 0.4), -9.185);
        }
        else if (kintern >= 0.1)
        {
            factor = 0.1 * pow(1. - (1.5859 - (1.3993 / pow(kintern, 0.054381))) * pow((ec * Cu) / P, 0.4), -9.185);
        }
        break;
    case AXIAAL_KOGEL:
        if(kintern >= 1.0)
        {
            factor = 0.1 * pow(1. - pow(2.5671 - (1.9987 / pow(kintern, 0.071739)), 0.83) * pow((ec * Cu) / (3. * P), 1. / 3.), -9.3);
        }
        else if (kintern >= 0.4)
        {
            factor = 0.1 * pow(1. - pow(2.5671 - (1.9987 / pow(kintern, 0.19087)), 0.83) * pow((ec * Cu) / (3. * P), 1. / 3.), -9.3);
        }
        else if (kintern >= 0.1)
        {
            factor = 0.1 * pow(1. - pow(2.5671 - (2.2649 / pow(kintern, 0.054381)), 0.83) * pow((ec * Cu) / (3. * P), 1. / 3.), -9.3);
        }
        break;
    case AXIAAL_CILINDER:
        if(kintern >= 1.0)
        {
            factor = 0.1 * pow(1. - (1.5859 - (1.2348 / pow(kintern, 0.071739))) * pow((ec * Cu) / (2.5 * P), 0.4), -9.185);
        }
        else if (kintern >= 0.4)
        {
            factor = 0.1 * pow(1. - (1.5859 - (1.2348 / pow(kintern, 0.19087))) * pow((ec * Cu) / (2.5 * P), 0.4), -9.185);
        }
        else if (kintern >= 0.1)
        {
            factor = 0.1 * pow(1. - (1.5859 - (1.3993 / pow(kintern, 0.054381))) * pow((ec * Cu) / (2.5 * P), 0.4), -9.185);
        }
        break;
    }

    // Factor mag niet groter zijn dan 50
    if (factor > 50.0)
    {
        factor = 50.0;
    }
    return factor;
}

/**
 * @brief Berekent de viscositeit bij de gekozen temperatuur aan de hand van een referentieviscositeit bij 40°C bij een minerale olie met een VI van 95
 * 
 * @param referentieviscositeit De basisviscositeit bij 40°C
 * @param werkingstemperatuur   De werkingstemperatuur waarbij men de viscositeit wilt weten in °C
 * @return double 
 */
double viscositeitOpTemperatuur(double referentieviscositeit, double werkingstemperatuur)
{
    double berekendeViscositeit = 0;

    double rechterlid = log10(log10(referentieviscositeit + 0.7)) + ((log(referentieviscositeit/384692722336.44))/6.33229857627451) * log10((werkingstemperatuur+273.0)/(40.0+273.0));

    berekendeViscositeit = pow(10, pow(10, rechterlid)) - 0.7;

    return berekendeViscositeit; 
}


struct samengesteldeBelasting_argumenten
{
    double statischeKracht;
    double dynamischeKracht;
    double exponent;
};

/**
 * @brief Interne functie die nodig is om de integraal voor de samengestelde belasting te berekenen
 * Ontvangt drie extra argumenten in de array
 * argumenten[0] = statische kracht
 * argumenten[1] = dynamische kracht of centrifugaalkracht
 * argumenten[2] = exponent (3 voor kogellagers; 10/3 voor rollagers)
 * @param x Veranderlijke in functie van de tijd
 * @param argumenten Extra argumenten (zie hiervoven)
 * @return double
 */
static double samengesteldeBelasting_formule_integraal(double x, void* argumenten)
{
    struct samengesteldeBelasting_argumenten* parameters = (struct samengesteldeBelasting_argumenten*)argumenten;

    double statisch = parameters->statischeKracht;
    double dynamisch = parameters->dynamischeKracht;
    double exponent = parameters->exponent;

    return pow(sqrt(pow(dynamisch * sin(x) + statisch , 2.0) + pow(dynamisch * cos(x) , 2.0)), exponent);
}

/**
 * @brief Berkent de equivalente belasting bij een samengestelde belasting van een statische en dynamische kracht
 * 
 * @param statisch Statische belasting in Newton
 * @param dynamisch Dynamische belasting of centrifugaalkracht in Newton
 * @param exponent Exponent 3 of 3,33
 * @return double 
 */
double samengesteldeBelasting(double statisch, double dynamisch, double exponent)
{
    // Struct maken om argumenten voor de berekening door te geven.
    struct samengesteldeBelasting_argumenten argumenten = {statisch, dynamisch, exponent};
    
    // Variabelen aanmaken om het resultaat en de geschatte fout bij de berekening in op te slaan.
    double resultaatIntegraal, error;
    // Werkruimte aanmaken voor de berekening
    gsl_integration_workspace* werkruimte = gsl_integration_workspace_alloc(1024); //?
    
    // GSL_Functie aanmaken waarop de berekening wordt uitgevoerd.
    // Bestaat uit de formule die de belasting op het lager op elk punt gedurende de omwenteling weergeeft en de argumenten
    gsl_function F;
    F.function = &samengesteldeBelasting_formule_integraal;
    F.params = &argumenten;

    // Integraal berekenen
    gsl_integration_qag(&F, 0, 2* M_PI, 0, 1e-7, 1024, 1, werkruimte, &resultaatIntegraal, &error);

    gsl_integration_workspace_free(werkruimte);
    // Equivalente belasting van de omwenteling berekenen
    double resultaat = pow(resultaatIntegraal / (2 * M_PI) , 1.0/exponent);

    return resultaat;
}

double NodigeViscositeit(double gemiddeldediameter, double toerental)
{
    if(toerental < 1000)
    {
        return 45000 * pow(toerental, -0.83) * pow(gemiddeldediameter, -0.5);
    }
    else // groter of gelijk aan 1000
    {
        return 4500 * pow(toerental, -0.5) * pow(gemiddeldediameter, -0.5);
    }
}


static bool ExactZoeken_EersteRij = true;
static bool ExactZoeken_EersteCel = true;
static bool ExactZoeken_JuisteRij = false;
static bool ExactZoeken_GegevenGevonden = false;
static size_t ExactZoeken_AantalKolommen = 0;
static int ExactZoeken_HuidigeKolom = 0;
static char* ExactZoeken_Zoekterm;
static char** ExactZoeken_GevondenGegevens;
static char** ExactZoeken_KolomKoppen;

/**
 * @brief Wordt uitgevoerd bij elke cel in het csv bestand
 * Interne functie van exact zoeken
 * @param gegeven De uitgelezen string in de cel
 * @param lengte De lengte van de string (exclusief 0 op het einde)
 * @param datatabel Pointer naar de volledige tabel waaruit het gegeven kwam
 */
static void ExactZoeken_EindeCel(void* gegeven, size_t lengte, __attribute__ ((unused)) void* datatabel) // Lengte zonder 0x00 op het einde
{
    if(ExactZoeken_EersteCel)
    {
        ExactZoeken_EersteCel = false;

        if(!strcmp((char*)gegeven, ExactZoeken_Zoekterm))
        {
            // De eerste cel komt overeen met het nodige gegeven
            ExactZoeken_JuisteRij = true; 
            ExactZoeken_GegevenGevonden = true;
        }
    }
    
    // Aantal kolommenTellen
    if(ExactZoeken_EersteRij)
    {
        ExactZoeken_AantalKolommen++;

        // Kolomkoppen opslaan
        // Omdat er nog niet exact is geweten hoeveel kolommen er in totaal zijn, moet de array met koppen steeds worden uitgebreid.
        if(ExactZoeken_KolomKoppen == NULL)
        {
            ExactZoeken_KolomKoppen = (char**)calloc(ExactZoeken_AantalKolommen, sizeof(char*));
        }
        else
        {
            ExactZoeken_KolomKoppen = realloc(ExactZoeken_KolomKoppen, sizeof(char*) * ExactZoeken_AantalKolommen);
        }

        // Toewijzen
        ExactZoeken_KolomKoppen[ExactZoeken_AantalKolommen - 1] = calloc(lengte + 1, sizeof(char));
        strcpy(ExactZoeken_KolomKoppen[ExactZoeken_AantalKolommen - 1], gegeven);
    }

    if(ExactZoeken_JuisteRij)
    {
        // Indien het de juiste rij is, moet het gegeven worden opgeslagen
        ExactZoeken_GevondenGegevens[ExactZoeken_HuidigeKolom] = (char*)calloc(lengte + 1, sizeof(char)); // De eerste rij wordt nooit gevraagd, want deze bevat de kolomkoppen Bijgevolg zal de eerste pointer dus altijd al zijn toegewezen
        strcpy(ExactZoeken_GevondenGegevens[ExactZoeken_HuidigeKolom], (char*)gegeven);

        ExactZoeken_HuidigeKolom++;
    }
}

/**
 * @brief Wordt op het einde van elke rij van het CSV bestand uitgevoerd
 * Interne functie van exact zoeken
 * @param teken het teken dat het einde van de rij weergeeft
 * @param datatabel De pointer naar de volledige datatabel
 */
static void ExactZoeken_EindeRij(__attribute__ ((unused)) int teken, __attribute__ ((unused)) void* datatabel)
{
    if(ExactZoeken_EersteRij)
    {
        ExactZoeken_EersteRij = false;

        // Nadat de eerste rij is overlopen, moet het geheugen worden gereserveerd voor de mogelijke gegevens die erin moeten
        ExactZoeken_GevondenGegevens = (char**)calloc(ExactZoeken_AantalKolommen, sizeof(char*));
    }

    ExactZoeken_HuidigeKolom = 0;
    ExactZoeken_EersteCel = true;
    ExactZoeken_JuisteRij = false;
}

/**
 * @brief Gaat zoeken naar een exacte rij waarbij de eerste cel exact overeenkomt met de ingevoerde zoekterm.
 * Geeft NULL terug indien er niets is gevonden, anders geeft het elke individuele cel terug
 * 
 * @param zoekterm De string die men wilt zoeken
 * @return char** 
 */
static lagerinformatie ExactZoeken_Intern(char* zoekterm, unsigned char* tabel, unsigned int groottetabel)
{
    ExactZoeken_Zoekterm = zoekterm;

    struct csv_parser p;
    csv_init(&p, CSV_APPEND_NULL);
    csv_set_delim(&p, ';');

    ExactZoeken_EersteRij = true;
    ExactZoeken_EersteCel = true;
    ExactZoeken_JuisteRij = false;
    ExactZoeken_GegevenGevonden = false;
    ExactZoeken_AantalKolommen = 0;
    ExactZoeken_HuidigeKolom = 0;
    ExactZoeken_GevondenGegevens = NULL;
    ExactZoeken_KolomKoppen = NULL;

    lagerinformatie informatie;

    csv_parse(&p, tabel, groottetabel, ExactZoeken_EindeCel, ExactZoeken_EindeRij, NULL);
    csv_fini(&p, ExactZoeken_EindeCel, ExactZoeken_EindeRij, NULL);
    csv_free(&p);

    if(ExactZoeken_GegevenGevonden)
    {
        informatie.aantalGegevens = ExactZoeken_AantalKolommen;
        informatie.kolomtitels = ExactZoeken_KolomKoppen;
        informatie.lagergegevens = ExactZoeken_GevondenGegevens;
        return informatie;
    }
    else
    {
        informatie.aantalGegevens = 0;
        informatie.kolomtitels = NULL;
        informatie.lagergegevens = NULL;
        return informatie;
    }
}

lagerinformatie ExactZoeken(char* zoekterm)
{
    // In elke tabel kijken of er een lager wordt gevonden
    lagerinformatie lager;

    lager = ExactZoeken_Intern(zoekterm, carblager_csv, carblager_csv_len); // carblager
    if(lager.aantalGegevens == 0) // Cilinderlager
    {
        lager = ExactZoeken_Intern(zoekterm, cilinderlagers_csv, cilinderlagers_csv_len);
        lager.lagersoort = LAGERSOORT_CILINDERLAGER;
    }
    if(lager.aantalGegevens == 0) // Dubbelrijghoekcontactkogellager
    {
        lager = ExactZoeken_Intern(zoekterm, dubbelrijighoekcontactkogellagers_csv, dubbelrijighoekcontactkogellagers_csv_len);
        lager.lagersoort = LAGERSOORT_DUBBELRIJIGHOEKCONTACTKOGELLAGER;
    }
    if(lager.aantalGegevens == 0) // Dubbelrijigkogellager
    {
        lager = ExactZoeken_Intern(zoekterm, dubbelrijigkogellager_csv, dubbelrijigkogellager_csv_len);
        lager.lagersoort = LAGERSOORT_DUBBELRIJIGKOGELLAGER;
    }
    if(lager.aantalGegevens == 0) // Dubbelrijigtonlager
    {
        lager = ExactZoeken_Intern(zoekterm, dubbelrijigtonlagers_csv, dubbelrijigtonlagers_csv_len);
        lager.lagersoort = LAGERSOORT_DUBBELRIJIGTONLAGER;
    }
    if(lager.aantalGegevens == 0) // Hoekcontactgollager
    {
        lager = ExactZoeken_Intern(zoekterm, hoekcontactkogellagers_csv, hoekcontactkogellagers_csv_len);
        lager.lagersoort = LAGERSOORT_HOEKCONTACTKOGELLAGER;
    }
    if(lager.aantalGegevens == 0) // Kegellager
    {
        lager = ExactZoeken_Intern(zoekterm, kegellagers_csv, kegellagers_csv_len);
        lager.lagersoort = LAGERSOORT_KEGELLAGER;
    }
    if(lager.aantalGegevens == 0) // Kogellager
    {
        lager = ExactZoeken_Intern(zoekterm, kogellagers_csv, kogellagers_csv_len);
        lager.lagersoort = LAGERSOORT_KOGELLAGER;
    }
    if(lager.aantalGegevens == 0) // Tonlager
    {
        lager = ExactZoeken_Intern(zoekterm, tonlagers_csv, tonlagers_csv_len);
        lager.lagersoort = LAGERSOORT_TONLAGER;
    }
    if(lager.aantalGegevens == 0) // Vierpuntslager
    {
        lager = ExactZoeken_Intern(zoekterm, vierpuntslagers_csv, vierpuntslagers_csv_len);
        lager.lagersoort = LAGERSOORT_VIERPUNTSLAGER;
    }
    if(lager.aantalGegevens == 0) // Zichinstellendkogellager
    {
        lager = ExactZoeken_Intern(zoekterm, zichinstellandekogellagers_csv, zichinstellandekogellagers_csv_len);
        lager.lagersoort = LAGERSOORT_ZICHINSTELLENDKOGELLAGER;
    }
    

    return lager;
}


static bool LagersZoeken_EersteCel = true;
static size_t LagersZoeken_AantalLagers = 0;
static char* LagersZoeken_Zoekterm;
static char** LagersZoeken_GevondenLagers;

static void LagersZoeken_EindeCel(void* gegeven, size_t lengte, __attribute__ ((unused)) void* datatabel)
{
    if(LagersZoeken_EersteCel && (strlen(LagersZoeken_Zoekterm) <= lengte))
    {
        LagersZoeken_EersteCel = false;

        // Controleren of de eerste letters overeenkomen
        bool zelfdeBegin = true;
        for(size_t i = 0; i < strlen(LagersZoeken_Zoekterm); i++)
        {
            if(LagersZoeken_Zoekterm[i] != ((char*)gegeven)[i])
            {
                zelfdeBegin = false;
                break;
            }
        }
        // Er is gecontroleerd of ze overeenkomen

        if(zelfdeBegin)
        {
            // Waarde opslaan
            LagersZoeken_AantalLagers++;
            if(LagersZoeken_GevondenLagers == NULL)
            {
                LagersZoeken_GevondenLagers = malloc(sizeof(char*));
            }
            else
            {
                LagersZoeken_GevondenLagers = realloc(LagersZoeken_GevondenLagers, sizeof(char*) * LagersZoeken_AantalLagers);
            }

            // Effectief toewijzen
            LagersZoeken_GevondenLagers[LagersZoeken_AantalLagers - 1] = calloc(sizeof(char), lengte + 1);
            strcpy(LagersZoeken_GevondenLagers[LagersZoeken_AantalLagers - 1], gegeven);

        }

    }
}

static void LagersZoeken_EindeRij(__attribute__ ((unused)) int teken, __attribute__ ((unused)) void* datatabel)
{
    LagersZoeken_EersteCel = true;
}

/**
 * @brief Gaat alle lagers zoeken en oplijsten die overeenkomen met de zoekterm
 * 
 * @param zoekterm String die men wilt zoeken
 * @return gevondenlagers 
 */
static gevondenlagers LagersZoeken_Intern(char* restrict zoekterm, unsigned char* restrict tabel, unsigned int groottetabel)
{
    // Lijst maken van alle lagers waarvan de eerste tekens overeenkomen met de zoekterm
    struct csv_parser p;
    csv_init(&p, CSV_APPEND_NULL);
    csv_set_delim(&p, ';');

    LagersZoeken_Zoekterm = zoekterm;
    LagersZoeken_EersteCel = true;
    LagersZoeken_AantalLagers = 0;
    LagersZoeken_GevondenLagers = NULL;

    csv_parse(&p, tabel, groottetabel, LagersZoeken_EindeCel, LagersZoeken_EindeRij, NULL);
    csv_fini(&p, LagersZoeken_EindeCel, LagersZoeken_EindeRij, NULL);
    csv_free(&p);

    gevondenlagers lagers;
    lagers.aantal = LagersZoeken_AantalLagers;
    lagers.lagers = LagersZoeken_GevondenLagers;

    return lagers;
}

gevondenlagers LagersZoeken(char* zoekterm)
{
    // In alle verschillende lagertabellen zoeken en deze dan bundelen
    gevondenlagers carblagers = LagersZoeken_Intern(zoekterm, carblager_csv, carblager_csv_len);
    gevondenlagers cilinderlagers = LagersZoeken_Intern(zoekterm, cilinderlagers_csv, cilinderlagers_csv_len);
    gevondenlagers dubbelrijighoekcontactkogellagers = LagersZoeken_Intern(zoekterm, dubbelrijighoekcontactkogellagers_csv, dubbelrijighoekcontactkogellagers_csv_len);
    gevondenlagers dubbelrijigkogellagers = LagersZoeken_Intern(zoekterm, dubbelrijigkogellager_csv, dubbelrijigkogellager_csv_len);
    gevondenlagers dubbelrijigtonlagers = LagersZoeken_Intern(zoekterm, dubbelrijigtonlagers_csv, dubbelrijigtonlagers_csv_len);
    gevondenlagers hoekcontactkogellagers = LagersZoeken_Intern(zoekterm, hoekcontactkogellagers_csv, hoekcontactkogellagers_csv_len);
    gevondenlagers kegellagers = LagersZoeken_Intern(zoekterm, kegellagers_csv, kegellagers_csv_len);
    gevondenlagers kogellagers = LagersZoeken_Intern(zoekterm, kogellagers_csv, kogellagers_csv_len);
    gevondenlagers tonlagers = LagersZoeken_Intern(zoekterm, tonlagers_csv, tonlagers_csv_len);
    gevondenlagers vierpuntslagers = LagersZoeken_Intern(zoekterm, vierpuntslagers_csv, vierpuntslagers_csv_len);
    gevondenlagers zichinstellandekogellagers = LagersZoeken_Intern(zoekterm, zichinstellandekogellagers_csv, zichinstellandekogellagers_csv_len);

    // totaal aantal elementen bepalen
    gevondenlagers alleLagers;
    alleLagers.aantal = carblagers.aantal + cilinderlagers.aantal + dubbelrijighoekcontactkogellagers.aantal + dubbelrijigkogellagers.aantal + dubbelrijigtonlagers.aantal + hoekcontactkogellagers.aantal + kegellagers.aantal + kogellagers.aantal + tonlagers.aantal + vierpuntslagers.aantal + zichinstellandekogellagers.aantal;
    alleLagers.lagers = calloc(alleLagers.aantal, sizeof(char*));

    // Alle gevonden lagers overzetten naar de nieuwe array
    int teller = 0;
    // carblagers
    for(size_t i = 0; i < carblagers.aantal; i++)
    {
        alleLagers.lagers[teller] = carblagers.lagers[i];
        teller++;
    }
    // Cilinderlagers
    for(size_t i = 0; i < cilinderlagers.aantal; i++)
    {
        alleLagers.lagers[teller] = cilinderlagers.lagers[i];
        teller++;
    }
    // Dubbelrijighoekcontactkogellagers
    for(size_t i = 0; i < dubbelrijighoekcontactkogellagers.aantal; i++)
    {
        alleLagers.lagers[teller] = dubbelrijighoekcontactkogellagers.lagers[i];
        teller++;
    }
    // Dubbelrijigkogellager
    for(size_t i = 0; i < dubbelrijigkogellagers.aantal; i++)
    {
        alleLagers.lagers[teller] = dubbelrijigkogellagers.lagers[i];
        teller++;
    }
    // Dubbelrijigtonlagers
    for(size_t i = 0; i < dubbelrijigtonlagers.aantal; i++)
    {
        alleLagers.lagers[teller] = dubbelrijigtonlagers.lagers[i];
        teller++;
    }
    // hoekcontactkogellagers
    for(size_t i = 0; i < hoekcontactkogellagers.aantal; i++)
    {
        alleLagers.lagers[teller] = hoekcontactkogellagers.lagers[i];
        teller++;
    }
    // kegellagers
    for(size_t i = 0; i < kegellagers.aantal; i++)
    {
        alleLagers.lagers[teller] = kegellagers.lagers[i];
        teller++;
    }
    // kogellagers
    for(size_t i = 0; i < kogellagers.aantal; i++)
    {
        alleLagers.lagers[teller] = kogellagers.lagers[i];
        teller++;
    }
    // tonlagers
    for(size_t i = 0; i < tonlagers.aantal; i++)
    {
        alleLagers.lagers[teller] = tonlagers.lagers[i];
        teller++;
    }
    // vierpuntslagers
    for(size_t i = 0; i < vierpuntslagers.aantal; i++)
    {
        alleLagers.lagers[teller] = vierpuntslagers.lagers[i];
        teller++;
    }
    // zichtinstellendekogellagers
    for(size_t i = 0; i < zichinstellandekogellagers.aantal; i++)
    {
        alleLagers.lagers[teller] = zichinstellandekogellagers.lagers[i];
        teller++;
    }

    // oude lagergegevens opruimen. Hierbij moet enkel de char** worden opgeruimde, maar niet elke individuele char* want deze worden hergebruikt
    free(carblagers.lagers);
    free(cilinderlagers.lagers);
    free(dubbelrijighoekcontactkogellagers.lagers);
    free(dubbelrijigkogellagers.lagers);
    free(dubbelrijigtonlagers.lagers);
    free(hoekcontactkogellagers.lagers);
    free(kegellagers.lagers);
    free(kogellagers.lagers);
    free(tonlagers.lagers);
    free(vierpuntslagers.lagers);
    free(zichinstellandekogellagers.lagers);
    

    return alleLagers;
}

char* LagersoortNaarString(enum lagersoort lager)
{
    switch(lager)
    {
    case LAGERSOORT_KOGELLAGER:
        return "Kogellager";
    case LAGERSOORT_DUBBELRIJIGKOGELLAGER:
        return "Dubbelrijigkogellager";
    case LAGERSOORT_HOEKCONTACTKOGELLAGER:
        return "Hoekcontactkogellager";
    case LAGERSOORT_DUBBELRIJIGHOEKCONTACTKOGELLAGER:
        return "Dubbelrijighoekcontactkogellager";
    case LAGERSOORT_VIERPUNTSLAGER:
        return "Vierpuntslager";
    case LAGERSOORT_ZICHINSTELLENDKOGELLAGER:
        return "Zichinstellendkogellager";
    case LAGERSOORT_CILINDERLAGER:
        return "Cilinderlager";
    case LAGERSOORT_KEGELLAGER:
        return "Kegellager";
    case LAGERSOORT_TONLAGER:
        return "Tonlager";
    case LAGERSOORT_DUBBELRIJIGTONLAGER:
        return "Dubbelrijig Tonlager";
    case LAGERSOORT_CARBLAGER:
        return "Carblager";
    default:
        return NULL;
    }
}

void free_gevondenlagers(gevondenlagers* lagers)
{
    if(lagers->aantal!= 0)
    {
        for(size_t i = 0; i < lagers->aantal; i++)
        {
            free(lagers->lagers[i]);
        }
        free(lagers->lagers);
    }

    lagers->aantal = 0;
    lagers->lagers = NULL;

}

void free_lagerinformatie(lagerinformatie* lager)
{
    if (lager->aantalGegevens != 0)
    {
        for (size_t i = 0; i < lager->aantalGegevens; i++)
        {
            free(lager->lagergegevens[i]);
            free(lager->kolomtitels[i]);
        }
        free(lager->lagergegevens);
        free(lager->kolomtitels);
        //free(lager.typelager);// is geen malloc string, maar een constante ergens in het geheugen
    }

    lager->aantalGegevens = 0;
    lager->kolomtitels = NULL;
    lager->lagergegevens = NULL;
}

static double equivalenteBelasting_Kogellager(double radiaalkracht, double axiaalkracht, double f0, double statischdraaggetal)
{
    double resultaat;

    // Indien de verhouding tussen de axiale kracht en de radiale kracht kleiner is of gelijk aan e, dan is de equivalente belasting gelijk aan de radiale kracht, anders is het de som van de twee maal een bepaalde factor

    double factor = (f0 * axiaalkracht) / statischdraaggetal;
    // E kan als volgt worden benaderd
    double e = 0.2853 * pow(factor, 0.2248);

    if((axiaalkracht/radiaalkracht) <= e)
    {
        resultaat = radiaalkracht;
    }
    else
    {
        double X = 0.56;
        // Y-waarde kan als volgt worden benaderd
        double Y = 1.5346* pow(factor, -0.23);

        resultaat = X * radiaalkracht + Y * axiaalkracht;
    }


    return resultaat;
}

/**
 * @brief Veronderstel X of O opstelling
 * 
 * @param radiaalkracht 
 * @param axiaalkracht 
 * @return double 
 */
static double equivalenteBelasting_Hoekcontactkogellager(double radiaalkracht, double axiaalkracht)
{
    double resultaat = 0;
    // Indien de verhouding tussen de axiale en radiale kracht kleiner is dan 1,14 is de equivalente belasting gelijk aan de radiale kracht
    if((axiaalkracht/radiaalkracht) <= 1.14)
    {
        resultaat = radiaalkracht;
    }
    else
    {
        resultaat = 0.35 * radiaalkracht + 0.57 * axiaalkracht;
    }

    return resultaat;
}

static double equivalenteBelasting_Dubbelrijighoekcontactkogellager(double radiaalkracht, double axiaalkracht, int contacthoek)
{
    double resultaat = 0;
    switch(contacthoek)
    {
        case 25:
            if((axiaalkracht/radiaalkracht) <= 0.68)
            {
                resultaat = radiaalkracht + 0.92 * axiaalkracht;
            }
            else
            {
                resultaat = 0.67 * radiaalkracht + 1.41 * axiaalkracht;
            }
        break;
        case 30:
            if((axiaalkracht/radiaalkracht) <= 0.8)
            {
                resultaat = radiaalkracht + 0.78 * axiaalkracht;
            }
            else
            {
                resultaat = 0.63 * radiaalkracht + 1.24 * axiaalkracht;
            }
        break;
        case 35:
            if((axiaalkracht/radiaalkracht) <= 0.95)
            {
                resultaat = radiaalkracht + 0.66 * axiaalkracht;
            }
            else
            {
                resultaat = 0.6 * radiaalkracht + 1.07 * axiaalkracht;
            }
        break;
        case 45:
            if((axiaalkracht/radiaalkracht) <= 1.34)
            {
                resultaat = radiaalkracht + 0.47 * axiaalkracht;
            }
            else
            {
                resultaat = 0.54 * radiaalkracht + 0.81 * axiaalkracht;
            }
        break;
    }

    return resultaat;
}

static double equivalenteBelasting_Vierpuntslager(double radiaalkracht, double axiaalkracht)
{
    double resultaat = 0;

    if(radiaalkracht == 0)
    {
        resultaat = 1.07 * axiaalkracht;
    }
    else if((axiaalkracht/radiaalkracht) <= 0.95)
    {
        resultaat = radiaalkracht + 0.66 * axiaalkracht;
    }
    else
    {
        resultaat = 0.6 * radiaalkracht + 1.07 * axiaalkracht;
    }

    return resultaat;
}

static double equivalenteBelasting_ZichInstellendKogellager(double radiaalkracht, double axiaalkracht, double e, double Y1, double Y2)
{
    double resultaat = 0;

    if((axiaalkracht/radiaalkracht) <= e)
    {
        resultaat = radiaalkracht + Y1 * axiaalkracht;
    }
    else
    {
        resultaat = 0.65 * radiaalkracht + Y2 * axiaalkracht;
    }

    return resultaat;
}

static double equivalenteBelasting_Cilinderlager(double radiaalkracht, double axiaalkracht, char* lagernaam)
{
    double resultaat;
    double e;
    double Y;
    // Indien het lager begint met NJ2, NUP2, NJ3, NUP3 of NJ4, is e 0.2 en Y 0.6
    // Indine het lager begint met NJ22, NUP22, NJ23 of NUP23, is e 0.3 en Y 0.4
    if((strstr(lagernaam, "NJ22") != NULL) ||  (strstr(lagernaam, "NUP22") != NULL) || (strstr(lagernaam, "NJ23") != NULL) || (strstr(lagernaam, "NUP23") != NULL))
    {
        e = 0.3;
        Y = 0.4;
    }
    else if((strstr(lagernaam, "NJ2") != NULL) ||  (strstr(lagernaam, "NUP2") != NULL) || (strstr(lagernaam, "NJ3") != NULL) || (strstr(lagernaam, "NUP3") != NULL) || (strstr(lagernaam, "NJ4") != NULL))
    {
        e = 0.2;
        Y = 0.6;
    }
    else
    {
        // Geen lager met spoorkraag
        e = 100; // Axiale kracht kan niet worden opgevangen dus berekening is niet relevant
        Y = 0;
    }

    if((axiaalkracht/radiaalkracht) <= e)
    {
        resultaat = radiaalkracht;
    }
    else
    {
        resultaat = 0.92 * radiaalkracht + Y * axiaalkracht;
    }

    return resultaat;    
}

static double equivalenteBelasting_Kegellager(double radiaalkracht, double axiaalkracht, double e, double Y)
{
    double resultaat = 0;

    if((axiaalkracht/radiaalkracht) <= e)
    {
        resultaat = radiaalkracht;
    }
    else
    {
        resultaat = 0.4 * radiaalkracht + Y * axiaalkracht;
    }


    return resultaat;
}

static double equivalenteBelasting_Tonlager(double radiaalkracht, double axiaalkracht)
{
    return (radiaalkracht + 9.5 * axiaalkracht);
}

static double equivalenteBelasting_DubbelrijigTonlager(double radiaalkracht, double axiaalkracht, double e, double Y1, double Y2)
{
    double resultaat;

    if((axiaalkracht/radiaalkracht) <= e)
    {
        resultaat = radiaalkracht + Y1 * axiaalkracht;
    }
    else
    {
        resultaat = 0.67 * radiaalkracht + Y2 * axiaalkracht;
    }

    return resultaat;
}

static double equivalenteBelasting_Carblager(double radiaalkracht)
{
    return radiaalkracht;
}

double equivalenteBelasting(lagerinformatie lager, double radiaalkracht, double axiaalkracht)
{
    double resultaat = 0;
    
        
    switch(lager.lagersoort)
    {
        case LAGERSOORT_KOGELLAGER:
        {
            // Nodige gegevens uitlezen om de equivalente belasting te berekenen (f0 en statisch draaggetal)
            double f0 = atof(lager.lagergegevens[9]);
            double C0 = atof(lager.lagergegevens[5]);
            resultaat = equivalenteBelasting_Kogellager(radiaalkracht, axiaalkracht, f0, C0);
        }
        break;
        case LAGERSOORT_DUBBELRIJIGKOGELLAGER:
        {
            // Nodige gegevens uitlezen om de equivalente belasting te berekenen (f0 en statisch draaggetal)
            double f0 = atof(lager.lagergegevens[9]);
            double C0 = atof(lager.lagergegevens[5]);
            resultaat = equivalenteBelasting_Kogellager(radiaalkracht, axiaalkracht, f0, C0);
        }
        break;
        case LAGERSOORT_HOEKCONTACTKOGELLAGER:
        {
            resultaat = equivalenteBelasting_Hoekcontactkogellager(radiaalkracht, axiaalkracht);
        }
        break;
        case LAGERSOORT_DUBBELRIJIGHOEKCONTACTKOGELLAGER:
        {
            int contacthoek = atoi(lager.lagergegevens[9]);
            resultaat = equivalenteBelasting_Dubbelrijighoekcontactkogellager(radiaalkracht, axiaalkracht, contacthoek);
        }
        break;
        case LAGERSOORT_VIERPUNTSLAGER:
        {
            resultaat = equivalenteBelasting_Vierpuntslager(radiaalkracht, axiaalkracht);
        }
        break;
        case LAGERSOORT_ZICHINSTELLENDKOGELLAGER:
        {
            double e = atof(lager.lagergegevens[9]);
            double Y1 = atof(lager.lagergegevens[10]);
            double Y2 = atof(lager.lagergegevens[11]);
            resultaat = equivalenteBelasting_ZichInstellendKogellager(radiaalkracht, axiaalkracht, e, Y1, Y2);
        }
        break;
        case LAGERSOORT_CILINDERLAGER:
        {
            resultaat = equivalenteBelasting_Cilinderlager(radiaalkracht, axiaalkracht, lager.lagergegevens[0]);
        }
        break;
        case LAGERSOORT_KEGELLAGER:
        {
            double e = atof(lager.lagergegevens[11]);
            double Y = atof(lager.lagergegevens[12]);
            resultaat = equivalenteBelasting_Kegellager(radiaalkracht, axiaalkracht, e, Y);
        }
        break;
        case LAGERSOORT_TONLAGER:
        {
            resultaat = equivalenteBelasting_Tonlager(radiaalkracht, axiaalkracht);
        }
        break;
        case LAGERSOORT_DUBBELRIJIGTONLAGER:
        {
            double e = atof(lager.lagergegevens[9]);
            double Y1 = atof(lager.lagergegevens[10]);
            double Y2 = atof(lager.lagergegevens[11]);
            resultaat = equivalenteBelasting_DubbelrijigTonlager(radiaalkracht, axiaalkracht, e, Y1, Y2);
        }
        break;
        case LAGERSOORT_CARBLAGER:
        {
            resultaat = equivalenteBelasting_Carblager(radiaalkracht);
        }
        break;
    default:
        break;
    }


    return resultaat;
}


struct veranderlijkeBelasting_InterneArgumenten
{
    double (*toerentalfunctie)(double, void*);
    double (*belastingsfunctie)(double, void*);
    veranderlijkeBelasting_argumenten argumenten;
};

static double veranderlijkeBelasting_Functie_Intern(double x, void* argumenten)
{
    struct veranderlijkeBelasting_InterneArgumenten* parameters = argumenten;
    
    // Toerental bepalen:
    double toerental = parameters->toerentalfunctie(x, &parameters->argumenten);
    // Belasting bepalen:
    double belasting = parameters->belastingsfunctie(x, &parameters->argumenten);
    // controleren of er een aiso factor moet worden berekend:
    
    double aiso = 1.0;
    
    if(parameters->argumenten.aisoBerekenen == true)
    {
        // Aiso bepalen
        // Viscositeit bepalen
        double nodigeViscositeit = NodigeViscositeit(parameters->argumenten.gemiddeldediameter, toerental);
        double effectieveViscositeit = viscositeitOpTemperatuur(parameters->argumenten.referentieviscositeit, parameters->argumenten.werkingstemperatuur);
        double smeringsverhouding = effectieveViscositeit/nodigeViscositeit;
        aiso = aiso_correctiefactor(parameters->argumenten.lagertype, parameters->argumenten.properheid, parameters->argumenten.fatigueloadlimit, belasting, smeringsverhouding);
    }
    
    // Exponent bepalen
    double exponent = 3;
    if(parameters->argumenten.lagertype == RADIAAL_CILINDER || parameters->argumenten.lagertype == AXIAAL_CILINDER)
    {
        exponent = (10.0/3.0);
    }

    return ((1/aiso)*toerental*pow(belasting, exponent));
}

double veranderlijkeBelasting_Functie(double (*toerentalfunctie)(double, void*), double (*belastingsfunctie)(double, void*), double ondergrens, double bovengrens, veranderlijkeBelasting_argumenten argemunten, double* uitkomstgemiddeldtoerental)
{
    double resultaat = 0;

    // Totaal aantal omwentelingen berekenen in ((n/min)*s)
    double aantalOmwentelingen, error;

    // Berekenen van integraal
    // Werkruimte aanmaken
    gsl_integration_workspace* werkruimte = gsl_integration_workspace_alloc(1024);

    // GSL functie aanmaken. Geen parameter voor het toerental
    gsl_function F;
    F.function = toerentalfunctie;
    F.params = &argemunten;

    // Integraal berekenen
    gsl_integration_qag(&F, ondergrens, bovengrens, 0, 1e-7, 1024, 1, werkruimte, &aantalOmwentelingen, &error);


    // Bovenste lid functie berekenen
    double resultaatBovenstelid;

    // GSl kan worden hergebruikt
    // struct maken
    struct veranderlijkeBelasting_InterneArgumenten interneargumenten;
    interneargumenten.argumenten = argemunten;
    interneargumenten.toerentalfunctie = toerentalfunctie;
    interneargumenten.belastingsfunctie = belastingsfunctie;
    
    // Functie gereedmaken
    F.function = &veranderlijkeBelasting_Functie_Intern;
    F.params = &interneargumenten;

    // Integraal berekenen. Oude werkruimte wordt hergebruikt
    gsl_integration_qag(&F, ondergrens, bovengrens, 0, 1e-7, 1024, 1, werkruimte, &resultaatBovenstelid, &error);
        
        
    // Werkruimte verwijderen
    gsl_integration_workspace_free(werkruimte);

    // Controleren of het gemiddeld toerental moet worden bepaald
    if(uitkomstgemiddeldtoerental != NULL)
    {
        double gemiddeldToerental = aantalOmwentelingen/(bovengrens-ondergrens);

        *uitkomstgemiddeldtoerental = gemiddeldToerental;
    }

    // Resultaat berekenen
    // Exponent bepalen
    double exponent = 3;
    if(argemunten.lagertype == RADIAAL_CILINDER || argemunten.lagertype == AXIAAL_CILINDER)
    {
        exponent = (10.0/3.0);
    }
    
    resultaat = pow(resultaatBovenstelid/aantalOmwentelingen, 1.0/exponent);


    return resultaat;
}

/**
 * @brief Geeft de geïnterpoleerde Y waarde terug
 * 
 * @param x1 X-waarde 1 (punt 1)
 * @param y1 Y-waarde 1 (punt 1)
 * @param x2 X-waarde 2 (punt 2)
 * @param y2 Y-waarde 2 (punt 2)
 * @param xvraag X-waarde waarvan de Y-waarde moet worden berekend
 * @return double 
 */
double interpoleer(double x1, double y1, double x2, double y2, double xvraag)
{
    return (y2 - y1)/(x2 - x1)*(xvraag - x1) + y1;
}

struct veranderlijkeBelasting_procesgegevens* Procesverloop;
int AantalGegevens;

static double veranderlijkeBelasting_Tabel_Toerentalfunctie(double x, void* argumenten)
{
    // Pakt dat de grenzen [100;120[ zijn inclusieve ondergrens
    // De gegevens bestaan uit vakken. Het aantal vakken is één minder dan het aantal rijen. 
    // Binnen de boven en ondergrens van zo een vak moet er worden geïnterpoleerd. Vermits er wordt vanuit gegaan dat het een lineair verband is
    
    // Eerst moet er worden bepaald in wel vak men zich bevindt
    int vak = 0;
    while (x > Procesverloop[vak + 1].tijdstip)
    {
        // zolang de ingevulde tijd groter is dan de bovengrens van het vak, moet men het vak vermeerderen 
        vak++;
    } // Er is bepaald in welk vak men zit

    // De waarde interpoleren die men nodig heeft

    return interpoleer(Procesverloop[vak].tijdstip, Procesverloop[vak].toerental, Procesverloop[vak+1].tijdstip, Procesverloop[vak + 1].toerental, x);
}

static double veranderlijkeBelasting_Tabel_Belastingsfunctie(double x, void* argumenten)
{
    // Pakt dat de grenzen [100;120[ zijn inclusieve ondergrens
    // De gegevens bestaan uit vakken. Het aantal vakken is één minder dan het aantal rijen. 
    // Binnen de boven en ondergrens van zo een vak moet er worden geïnterpoleerd. Vermits er wordt vanuit gegaan dat het een lineair verband is
    
    // Eerst moet er worden bepaald in wel vak men zich bevindt
    int vak = 0;
    while (x > Procesverloop[vak + 1].tijdstip)
    {
        // zolang de ingevulde tijd groter is dan de bovengrens van het vak, moet men het vak vermeerderen 
        vak++;
    } // Er is bepaald in welk vak men zit

    // De waarde interpoleren die men nodig heeft

    return interpoleer(Procesverloop[vak].tijdstip, Procesverloop[vak].belasting, Procesverloop[vak+1].tijdstip, Procesverloop[vak + 1].belasting, x);
}

double veranderlijkeBelasting_Tabel(struct veranderlijkeBelasting_procesgegevens* gegevens, int aantalgegevens, veranderlijkeBelasting_argumenten argemunten, double* uitkomstgemiddeldtoerental)
{
    // De procesgegevens opslaan in de global variabele
    Procesverloop = gegevens;
    AantalGegevens = aantalgegevens;

    // Onder en bovengrens bepalen
    double ondergrens = gegevens[0].tijdstip;
    double bovengrens = gegevens[aantalgegevens - 1].tijdstip;

    double gemiddeldToerental;
    double equivalenteBelasting = veranderlijkeBelasting_Functie(veranderlijkeBelasting_Tabel_Toerentalfunctie, veranderlijkeBelasting_Tabel_Belastingsfunctie, ondergrens, bovengrens, argemunten, &gemiddeldToerental);

    if(uitkomstgemiddeldtoerental != NULL)
    {
        *uitkomstgemiddeldtoerental = gemiddeldToerental;
    }
    
    return equivalenteBelasting;
}
