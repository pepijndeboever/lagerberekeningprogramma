#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdbool.h>

// Enum

/**
 * @brief Tijdsbasis waarmee men de levensduur wilt berekenen
 * 
 */
enum tijdsbasis
{
    UUR,
    DAGEN,
    JAREN
};

/**
 * @brief Type lager dat men gebruikt. Vooral voor aISO factor
 * 
 */
enum lagertype
{
    RADIAAL_KOGEL,
    RADIAAL_CILINDER,
    AXIAAL_KOGEL,
    AXIAAL_CILINDER
};

/**
 * @brief Specifiek soort lager dat men gebruikt
 * 
 */
enum lagersoort
{
    LAGERSOORT_KOGELLAGER,
    LAGERSOORT_DUBBELRIJIGKOGELLAGER,
    LAGERSOORT_HOEKCONTACTKOGELLAGER,
    LAGERSOORT_DUBBELRIJIGHOEKCONTACTKOGELLAGER,
    LAGERSOORT_VIERPUNTSLAGER,
    LAGERSOORT_ZICHINSTELLENDKOGELLAGER,
    LAGERSOORT_CILINDERLAGER,
    LAGERSOORT_KEGELLAGER,
    LAGERSOORT_TONLAGER,
    LAGERSOORT_DUBBELRIJIGTONLAGER,
    LAGERSOORT_CARBLAGER
};


// Struct

/**
 * @brief Argumenten die worden gebruikt bij het berekenen van de equivalente belasting bij een veranderlijke belasting en veranderlijk toerental
 *
 */
typedef struct veranderlijkeBelasting_argumenten
{
    enum lagertype lagertype; // Type lager waarvan men de belasting wilt berekenen
    bool aisoBerekenen; // Of men wel of niet rekening wilt houden met de aiso correctiefactor
    double referentieviscositeit; // Referentieviscositeit (40°C)
    double gemiddeldediameter; // Gemiddelde diameter van het lager in mm
    double werkingstemperatuur; // Temperatuur waarbij het lager werkt in °C
    double fatigueloadlimit; // Fatigue load limit Cu van het lager in N
    double properheid;  // Properheidsfactor ec
    void* overigeArgumenten; // Overige argumenten die men wilt doorgeven aan belastingsfunctie en toerentalfunctie
} veranderlijkeBelasting_argumenten;

/**
 * @brief Gevonden lagerinformatie over het gezochte lager
 * 
 */
typedef struct lagerinformatie
{
    unsigned long aantalGegevens; // Aantal gegevens die over het lager zijn gevonden
    enum lagersoort lagersoort;  // Specifiek soort lager dat is gevonden
    char** kolomtitels; // Titel van elk gegeven dat is gevonen
    char** lagergegevens; // Effectief gevonden gegevens
} lagerinformatie;

/**
 * @brief De gevonden lagers
 * 
 */
typedef struct gevondenlagers
{
    char** lagers; // Naam van alle gevonden lagers
    unsigned long aantal; // Aantal gevonden lagers
} gevondenlagers;

/**
 * @brief Alle gegevens die belangrijk zijn van het procesverloop om de equivalente belasting te bepalen
 * Wordt meestal in arrayvorm gebruikt om de verschillende gegevens op verschillende tijdstippen te kennen
 */
struct veranderlijkeBelasting_procesgegevens
{
    double tijdstip;
    double toerental;
    double belasting;
};


// Functies

/**
 * @brief Berekend de standaard lagerlevensduur in miljoen omwentelingen
 * 
 * @param dynamischdraaggetal Dynamisch draaggetal in Newton
 * @param equivalentebelasting Equivalente belasting in Newton
 * @param exponent 3 of 3,33
 * @return double 
 */
double standaardLevensduur(double dynamischdraaggetal, double equivalentebelasting, double exponent);

/**
 * @brief Gaat in functie van het toerental de levensduur omzetten in functie van aantal draaiuren, draaidagen of draaijaren.
 * 
 * @param levensduur Levensduur in miljoen omwentelingen
 * @param toerental Toerental in n/min
 * @param tijd Tijdsbasis: UUR, DAGEN, JAREN
 * @return double 
 */
double levensduurOpTijd(double levensduur, double toerental, enum tijdsbasis tijd);

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
double aiso_correctiefactor(enum lagertype lager, double ec, double Cu, double P, double k);

/**
 * @brief Berekent de viscositeit bij de gekozen temperatuur aan de hand van een referentieviscositeit bij 40°C bij een minerale olie met een VI van 100
 * Accuraat tot ongeveer 0,5
 * Zeker goed tussen 40°C en 100°C. Daarbuiten niet getest
 * @param referentieviscositeit De basisviscositeit bij 40°C
 * @param werkingstemperatuur   De werkingstemperatuur waarbij men de viscositeit wilt weten in °C
 * @return long double 
 */
double viscositeitOpTemperatuur(double referentieviscositeit, double werkingstemperatuur);

/**
 * @brief Berkent de equivalente belasting bij een samengestelde belasting van een statische en dynamische kracht
 * 
 * @param statisch Statische belasting in Newton
 * @param dynamisch Dynamische belasting of centrifugaalkracht in Newton
 * @param exponent Exponent 3 of 3,33
 * @return double 
 */
double samengesteldeBelasting(double statisch, double dynamisch, double exponent);

/**
 * @brief Berekent de nodige viscositeit voor de smering van het lager in werking aan de hand van de gemiddelde diameter en het toerental
 * 
 * @param gemiddeldediameter Gemiddelde diameter van het lager in mm (d+D)/2
 * @param toerental Het toerental waaraan het lager draait in n/min
 * @return De nodige viscositeit in het lager in mm²/s
 */
double NodigeViscositeit(double gemiddeldediameter, double toerental);

/**
 * @brief Gaat een lager zoeken, waarvan de designatie exact overeenkomt met de zoekterm
 * Geeft bij de lagerinformatie 0 en NULL terug indien er niets is gevonden dat exact overeenkomt.
 * BELANGRIJK: teruggekregen lagerinformatie moet worden vrijgemaakt met free_lagerinformatie()
 * @param zoekterm Naam van het lager dat men wilt zoeken
 * @return lagerinformatie 
 */
lagerinformatie ExactZoeken(char* zoekterm);

/**
 * @brief Gaat alle lagers zoeken waarvan de ingegeven zoekterm overeenkomt met het bevin van de designatie
 * 629 --> 629-C, 629-2Z...
 * BELANGRIJK: Teruggekragen lagers moeten worden vrijgemaakt met free_gevondenlagers()
 * @param zoekterm Begin van de naam van het lager dat men wilt zoeken
 * @return gevondenlagers 
 */
gevondenlagers LagersZoeken(char* zoekterm);

/**
 * @brief Zet het lagersoort om in een string om weer te geven
 * 
 * @param lager Lagersoort waarvan men de naam als string wilt
 * @return char* 
 */
char* LagersoortNaarString(enum lagersoort lager);

/**
 * @brief Gaat het gereserveerde geheugen vrijmaken voor de gevonden lagers
 * 
 * @param lagers De gevonden lagers die men uit het geheugen wilt verwijderen
 */
void free_gevondenlagers(gevondenlagers* lagers);

/**
 * @brief Gaat het gereserveerde geheugen vrijmaken voor de lagerinformatie
 * 
 * @param lager De lagerinformatie die men uit het geheugen wilt verwijderen
 */
void free_lagerinformatie(lagerinformatie* lager);

/**
 * @brief Berekent de equivalente belasting (P) van een lager dat wordt belast door een statische radiaalkracht en axiaalkracht in functie van het type lager dat is gekozen
 * @param lager Lager waarvan men de equivalente belasting (P) wilt weten
 * @param radiaalkracht Radiale kracht dat wordt uitgeoefend op het lager in Newton
 * @param axiaalkracht Axiale kracht dat op het lager wordt uitgeoefend in Newton
 * @return Equivalente belasting op het lager in Newton
 */
double equivalenteBelasting(lagerinformatie lager, double radiaalkracht, double axiaalkracht);

/**
 * @brief Gaat de equivalente belasting berekenen die op het lager wordt uitgeoefend gedurende het variabele proces waarbij zowel het toerental als de belasting kunnen varieren.
 * 
 * @param toerentalfunctie Functie die het toerental beschrijft in functie van de tijd (in n/min)
 * Moet als eerste parameter het tijdstip aanvaarden (double) waarvan men het toerental wilt weten en als tweede parameter een void*. Deze krijgt de struct veranderlijkeBelasting_argumenten mee en kan intern worden uitgelezen
 * @param belastingsfunctie Functie die de belasting op het lager beschrijft in functie van de tijd (in Newton)
 * Moet als eerste parameter het tijdstip aanvaarden (double) waarvan men de belasting wilt weten en als tweede parameter een void*. Deze krijgt de struct veranderlijkeBelasting_argumenten mee en kan intern worden uitgelezen
 * @param ondergrens Minimum tijdstip waarop men wilt beginnen (vaak 0) in seconden
 * @param bovengrens Maximale tijdstip waarbij men het wilt berekenen (vaak de hele procesduur) in seconden
 * @param argumenten Struct waarin men alle informatie plaatst die nodig is om de aiso factor te berekenen en welke exponent men moet gebruiken voor het lager
 * Wordt ook doorgegeven naar de toerentalfunctie en belastingsfunctie
 * @param uitkomstgemiddeldtoerental Hier kan men indien gewenst een pointer invullen en deze zal op het einde van de berekening worden ingevuld met het gemiddelde toerental in (n/min) van het proces
 * Indien men dit niet wenst te weten kan men NULL invullen
 * @return double Geeft de equivalente belasting terug die het lager ondervondt gedurende het variabele proces in Newton
 */
double veranderlijkeBelasting_Functie(double (*toerentalfunctie)(double, void*), double (*belastingsfunctie)(double, void*), double ondergrens, double bovengrens, veranderlijkeBelasting_argumenten argumenten, double* uitkomstgemiddeldtoerental);

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
double interpoleer(double x1, double y1, double x2, double y2, double xvraag);

/**
 * @brief Geeft de equivalente belasting die het lager ondervindt gedurende een variabel proces
 * De procesgegevens worden aan de hand van een struct array toegegeven. 
 * Hierbij moet men in de procesgegevens op bepaalde tijdstippen zeggen wat de belasting is en het toerental
 * De gegevens tussen twee tijdstippen worden door lineaire interpollatie bepaald
 * De gegevens moeten gesorteerd zijn op tijd
 * @param gegevens De array van procesgegevens. Deze bestaat uit verschillende momenten waarvan het tijdstip, toerental en belasting worden gegeven. Waarden tussen twee punten worden door linaire interpollatie bepaald
 * @param aantalgegevens De hoeveelheid punten waarvan men gegevens geeft, (moet overeenkomen met de lengte van de array. (laatste gegeven is gegevens[aantalgegevens - 1])
 * @param argemunten Struct van gegevens over het lager en over de werkingstoestand van het lager, zoals viscositeit en temperatuur. 
 * Wordt ook doorgegeven naar belastingsfunctie en toerentalfunctie
 * @param uitkomstgemiddeldtoerental Hier kan men indien gewenst een pointer invullen en deze zal op het einde van de berekening worden ingevuld met het gemiddelde toerental in (n/min) van het proces
 * Indien men dit niet wenst te weten kan men NULL invullen
 * @return double De equivalente belasting in Newton die het lager ondervondt gedurende het variabele proces
 */
double veranderlijkeBelasting_Tabel(struct veranderlijkeBelasting_procesgegevens* gegevens, int aantalgegevens, veranderlijkeBelasting_argumenten argemunten, double* uitkomstgemiddeldtoerental);
