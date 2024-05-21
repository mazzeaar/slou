#include "magic/magic.h"
#include "config.h"
namespace magic {
    void storeMagicsToCppFile(const std::string& name, const std::array<Magic, 64>& magics);

    u64 indexToU64(int index, int bits, u64 m);

    template <PieceType type>
    u64 findMagicNumber(int square, int bits, u64 mask);

    template <PieceType type>
    std::array<u64, 4096> generateAttackTable(int square, u64 mask);

    template <PieceType type>
    u64 getMask(int square);
    template <> u64 getMask<PieceType::bishop>(int square);
    template <> u64 getMask<PieceType::rook>(int square);

    template <PieceType type>
    u64 getAttackPattern(int square, u64 occupancy);

    static int RBits[numSquares] = {
      12, 11, 11, 11, 11, 11, 11, 12,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      12, 11, 11, 11, 11, 11, 11, 12
    };

    static int BBits[numSquares] = {
      6, 5, 5, 5, 5, 5, 5, 6,
      5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5,
      6, 5, 5, 5, 5, 5, 5, 6
    };

    /**
     * @brief generates a key for the attack table
     *
     * @param blockers
     * @param magic
     * @param bits
     * @return int
     */
    inline int generateKey(u64 blockers, u64 magic, int bits)
    {
        return (int) ((blockers * magic) >> (64 - bits));
    }

    void initSquareMagics(int square)
    {
        bishop_magics[square].mask = getMask<PieceType::bishop>(square);
        bishop_magics[square].magic = findMagicNumber<PieceType::bishop>(square, BBits[square], bishop_magics[square].mask);
        bishop_magics[square].shift = 64 - get_bit_count(bishop_magics[square].mask);
        bishop_magics[square].attack_table = generateAttackTable<PieceType::bishop>(square, bishop_magics[square].mask);

        rook_magics[square].mask = getMask<PieceType::rook>(square);
        rook_magics[square].magic = findMagicNumber<PieceType::rook>(square, RBits[square], rook_magics[square].mask);
        rook_magics[square].shift = 64 - get_bit_count(rook_magics[square].mask);
        rook_magics[square].attack_table = generateAttackTable<PieceType::rook>(square, rook_magics[square].mask);
    }

    void initMagics()
    {
        static bool local_initialized_check = false;
        if ( initialized_magics || local_initialized_check ) {
            return;
        }

        std::cout << FAT_LINE << '\n' <<
            "As the engine is compiling for the first time the magics will now be initialized.\n"
            << "They will be written to bishop_magics.cpp and rook_magics.cpp respectively.\n"
            << THIN_LINE << '\n'
            << "The process will take a few seconds...\n";

        auto begin = std::chrono::high_resolution_clock::now();
        for ( int i = 0; i < 64; ++i ) {
            initSquareMagics(i);
        }
        auto end = std::chrono::high_resolution_clock::now();

        storeMagicsToCppFile("bishop_magics", bishop_magics);
        storeMagicsToCppFile("rook_magics", rook_magics);

        std::ofstream meta_magic("../src/magic/meta_magic.cpp");

        meta_magic << "/**\n"
            << "*   DO NOT TOUCH!!\n"
            << "*\n"
            << "*   This file will be changed after compiling for the first time.\n"
            << "*   This way we only need to calculate our magic numbers once.\n"
            << "*\n"
            << "*/\n\n";

        meta_magic << "#include \"magic/magic.h\"\n"
            << "const bool magic::initialized_magics = true;\n";

        meta_magic.close();

        local_initialized_check = true;

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::cout << "Magics have now been initialized. (took " << duration << "ms)\n"
            << "Startup will be faster from now on <3\n\n"
            << "-- PLEASE RECOMPILE TO ENSURE A BUGFREE EXPERIENCE --\n\n\n";
    }


    template <PieceType type>
    u64 findMagicNumber(int square, int bits, u64 mask)
    {
        static_assert(type == PieceType::bishop || type == PieceType::rook && "Piece type is not supported!");

        constexpr u64 UNOCCUPIED = 0xFFFFFFFFFFFFFFFFULL;
        constexpr int maxAttempts = 100000000;

        const int num_bits = get_bit_count(mask);
        const int num_configurations = (1 << num_bits);

        std::array<u64, 4096> attack_patterns, blocker_configurations;
        std::array<u64, 4096> used = { UNOCCUPIED };

        for ( int i = 0; i < num_configurations; ++i ) {
            blocker_configurations[i] = indexToU64(i, num_bits, mask);
            attack_patterns[i] = getAttackPattern<type>(square, blocker_configurations[i]);
        }

        for ( int k = 0; k < maxAttempts; ++k ) {
            const u64 magic = random_u64_fewbits();

            bool is_valid = true;
            for ( int i = 0; i < num_configurations; i++ ) {
                const int key = generateKey(blocker_configurations[i], magic, bits);
                if ( used[key] == UNOCCUPIED ) {
                    used[key] = attack_patterns[i];
                }
                else if ( used[key] != attack_patterns[i] ) {
                    is_valid = false;
                    break;
                }
            }

            if ( is_valid ) {
                return magic;
            }

            used.fill(UNOCCUPIED);
        }

        throw std::runtime_error("failed to initialize magics :(");
        return 0ULL; // to make the compiler shut up
    }

    template <PieceType type>
    std::array<u64, 4096> generateAttackTable(int square, u64 mask)
    {
        static_assert(type == PieceType::bishop || type == PieceType::rook && "Piece type is not supported");

        int num_bits = get_bit_count(mask);
        int num_entries = 0b1 << num_bits; // 2^numBits possible blocker configurations

        auto attack_table = getMagics<type>(square).attack_table;
        for ( int i = 0; i < num_entries; i++ ) {
            const u64 blockers = indexToU64(i, num_bits, mask);
            const u64 attacks = getAttackPattern<type>(square, blockers);
            const int key = generateKey(blockers, getMagics<type>(square).magic, num_bits);

            attack_table[key] = attacks;
        }

        return attack_table;
    }

    u64 indexToU64(int index, int bits, u64 m)
    {
        u64 result = 0ULL;
        for ( int i = 0; i < bits; i++ ) {
            int lsb = pop_LSB(m);
            if ( index & (1 << i) ) {
                result |= single_bit_u64(lsb);
            }
        }

        return result;
    }

    template <>
    u64 getMask<PieceType::rook>(int square)
    {
        u64 result = 0ULL;
        const int rank = square / 8;
        const int file = square % 8;

        for ( int r = rank + 1; r <= 6; ++r ) result |= single_bit_u64(file + r * 8);
        for ( int r = rank - 1; r >= 1; --r ) result |= single_bit_u64(file + r * 8);
        for ( int f = file + 1; f <= 6; ++f ) result |= single_bit_u64(f + rank * 8);
        for ( int f = file - 1; f >= 1; --f ) result |= single_bit_u64(f + rank * 8);

        return result;
    }

    template <>
    u64 getMask<PieceType::bishop>(int square)
    {
        u64 result = 0ULL;
        const int rank = square / 8;
        const int file = square % 8;

        for ( int r = rank + 1, f = file + 1; r <= 6 && f <= 6; ++r, ++f )
            result |= single_bit_u64(f + r * 8);
        for ( int r = rank + 1, f = file - 1; r <= 6 && f >= 1; ++r, --f )
            result |= single_bit_u64(f + r * 8);
        for ( int r = rank - 1, f = file + 1; r >= 1 && f <= 6; --r, ++f )
            result |= single_bit_u64(f + r * 8);
        for ( int r = rank - 1, f = file - 1; r >= 1 && f >= 1; --r, --f )
            result |= single_bit_u64(f + r * 8);

        return result;
    }

    template <>
    u64 getAttackPattern<PieceType::rook>(int square, u64 occupancy)
    {
        u64 result = 0ULL;
        const int rank = square / 8;
        const int file = square % 8;
        int r, f;

        for ( r = rank + 1; r <= 7; r++ ) {
            result |= single_bit_u64(file + r * 8);
            if ( occupancy & single_bit_u64(file + r * 8) ) break;
        }

        for ( r = rank - 1; r >= 0; r-- ) {
            result |= single_bit_u64(file + r * 8);
            if ( occupancy & single_bit_u64(file + r * 8) ) break;
        }

        for ( f = file + 1; f <= 7; f++ ) {
            result |= single_bit_u64(f + rank * 8);
            if ( occupancy & single_bit_u64(f + rank * 8) ) break;
        }

        for ( f = file - 1; f >= 0; f-- ) {
            result |= single_bit_u64(f + rank * 8);
            if ( occupancy & single_bit_u64(f + rank * 8) ) break;
        }

        return result;
    }

    template <>
    u64 getAttackPattern<PieceType::bishop>(int square, u64 occupancy)
    {
        u64 result = 0ULL;
        const int rank = square / 8;
        const int file = square % 8;

        for ( int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++ ) {
            result |= single_bit_u64(f + r * 8);
            if ( occupancy & single_bit_u64(f + r * 8) ) break;
        }

        for ( int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f-- ) {
            result |= single_bit_u64(f + r * 8);
            if ( occupancy & single_bit_u64(f + r * 8) ) break;
        }

        for ( int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++ ) {
            result |= single_bit_u64(f + r * 8);
            if ( occupancy & single_bit_u64(f + r * 8) ) break;
        }

        for ( int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f-- ) {
            result |= single_bit_u64(f + r * 8);
            if ( occupancy & single_bit_u64(f + r * 8) ) break;
        }

        return result;
    }

    void storeMagicsToCppFile(const std::string& name, const std::array<Magic, 64>& magics)
    {
        std::ofstream file("../src/magic/" + name + ".cpp");

        file << "/**\n"
            << "*   DO NOT TOUCH!!\n"
            << "*\n"
            << "*   This file will be or is already filled by magic.h.\n"
            << "*\n"
            << "*   This file will be overwritten when compiling for the first time, this way we only compute the magics once.\n"
            << "*\n"
            << "*   It will take a few seconds to compute all the magic numbers.\n"
            << "*/\n\n";

        file << "#include \"magic/magic.h\"\n\n";
        file << "std::array<magic::Magic, 64> magic::" << name << " = {{\n";

        for ( int i = 0; i < 64; ++i ) {
            file << "    {";   // new Magic entry

            // print the attack table
            file << "\n    {{ ";
            for ( int j = 0; j < 4096; ++j ) {
                file << "0x" << std::hex << magics[i].attack_table[j];
                if ( j < 4095 ) file << ", ";
            }
            file << " }}, ";

            file << "0x" << std::hex << magics[i].mask << ", "      // mask
                << "0x" << std::hex << magics[i].magic << ", "      // magic
                << "0x" << std::hex << magics[i].shift << "";       // shift

            file << "\n    }";       // close Magic entry

            if ( i < 63 ) file << ",";
            file << std::endl;
        }

        file << "}};\n"; // close the array

        file.close();
    }

}; // namespace magic