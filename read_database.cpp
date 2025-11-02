#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <optional>
#include <stdexcept>
#include <cstdint>
// Lit une longueur BER sur le flux 'file'.
// Renvoie la longueur si OK, sinon std::nullopt (EOF/erreur).
std::optional<size_t> readBERLength(std::istream& file) {
    int c0 = file.get();                                // lit le premier octet
    if (c0 == EOF) return std::nullopt;               // fin de fichier -> échec

    unsigned char o0 = static_cast<unsigned char>(c0);// convertit en octet non signé

    if (o0 < 0x80) {                                  // CAS 1 : longueur courte
        return static_cast<size_t>(o0);               // cet octet EST la longueur
    } else {                                          // CAS 2 : longueur longue
        unsigned char k = o0 & 0x7F;                  // nb d'octets qui suivent
        size_t L = 0;                                  // longueur finale
        for (unsigned i = 0; i < k; ++i) {            // lit k octets de longueur
            int ci = file.get();                        // lit un octet
            if (ci == EOF) return std::nullopt;       // erreur si fin de fichier
            L = (L << 8) | static_cast<unsigned char>(ci); // concatène en big-endian
        }
        return L;                                      // renvoie la longueur décodée
    }
}


// Extrait le premier "mot" de la première VisibleString du header i.
// - phr      : flux ouvert sur .phr
// - hdrStart : offset de début du header i dans .phr
// - hdrEnd   : (optionnel) offset de fin du header i (sécurité pour le scan)
std::string firstWordFromPHR(std::istream& phr,
                             uint64_t hdrStart,
                             std::optional<uint64_t> hdrEnd = std::nullopt)
{
    phr.clear();                                                // réinitialise les flags du flux
    phr.seekg(static_cast<std::streamoff>(hdrStart), std::ios::beg); // se place à hdrStart

    // 1) Scanner jusqu'au TAG 0x1A (VisibleString)
    while (true) {
        int c = phr.get();                                      // lit un octet
        if (c == EOF) return {};                                // si fin -> rien trouvé
        if (static_cast<unsigned char>(c) == 0x1A) break;       // 0x1A trouvé -> on sort
        if (hdrEnd && static_cast<uint64_t>(phr.tellg()) >= *hdrEnd) // borne de sécurité
            return {};                                          // on quitte si on a dépassé
    }

    // 2) Juste après 0x1A, lire la longueur BER de la chaîne
    auto optLen = readBERLength(phr);                           // utilise la Tâche 2
    if (!optLen) return {};                                     // échec -> chaîne absente
    size_t L = *optLen;                                         // nombre d'octets à lire

    // 3) Lire exactement L octets : c'est la chaîne "defline"
    std::string defline(L, '\0');                               // prépare une string de taille L
    phr.read(defline.data(), static_cast<std::streamsize>(L));  // lit L octets dans la string

    // 4) Ne garder que le premier "mot" (avant l'espace)
    size_t posSpace = defline.find(' ');                        // cherche le premier espace
    if (posSpace == std::string::npos) return defline;          // pas d'espace -> renvoie tout
    return defline.substr(0, posSpace);                         // sinon renvoie [0..posSpace[
}
