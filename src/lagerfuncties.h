#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(func) func
#endif


enum tijdsbasis
{
    UUR,
    DAGEN,
    JAREN
};

enum lagertype
{
    RADIAAL_KOGEL,
    RADIAAL_CILINDER,
    AXIAAL_KOGEL,
    AXIAAL_CILINDER
};

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

double samengesteldeBelasting(double statisch, double dynamisch, double exponent);

double NodigeViscositeit(double gemiddeldediameter, double toerental);


typedef struct lagerinformatie
{
    unsigned long aantalGegevens;
    enum lagersoort lagersoort;
    char** kolomtitels;
    char** lagergegevens;
} lagerinformatie;

typedef struct gevondenlagers
{
    char** lagers;
    unsigned long aantal;
} gevondenlagers;

lagerinformatie ExactZoeken(char* zoekterm);

gevondenlagers LagersZoeken(char* zoekterm);

char* LagersoortNaarString(enum lagersoort lager);

void free_gevondenlagers(gevondenlagers* lagers);

void free_lagerinformatie(lagerinformatie* lager);
