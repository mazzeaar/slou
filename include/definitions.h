#pragma once

#include "log.h"

#include <cstdint>
#include <string>
#include <array>

namespace type {
    enum class Color {
        white, black, none
    };

    enum class PieceType {
        pawn, knight, bishop, rook, queen, king, none
    };

    enum class Piece {
        P, N, B, R, Q, K,
        p, n, b, r, q, k,
        none
    };

    constexpr uint8_t isBishop(PieceType type) { return type == PieceType::bishop; }
    constexpr uint8_t isRook(PieceType type) { return type == PieceType::rook; }
    constexpr uint8_t isQueen(PieceType type) { return type == PieceType::queen; }

    constexpr uint8_t toByte(Piece piece) { return static_cast<uint8_t>(piece); }
    constexpr uint8_t toByte(PieceType type) { return static_cast<uint8_t>(type); }
    constexpr uint8_t toByte(Color color) { return static_cast<uint8_t>(color); }

    constexpr Color getColor(Piece piece)
    {
        switch ( piece ) {
            case Piece::p: case Piece::n: case Piece::b: case Piece::r: case Piece::q: case Piece::k:
                return Color::black;
            case Piece::P: case Piece::N: case Piece::B: case Piece::R: case Piece::Q: case Piece::K:
                return Color::white;
            default:
                return Color::none;
        }
    }

    constexpr Color switchColor(Color color)
    {
        if ( color == Color::none ) {
            return Color::none;
        }
        else {
            return color == Color::white ? Color::black : Color::white;
        }
    }

    constexpr Piece switchColor(Piece piece)
    {
        switch ( piece ) {
            case Piece::p: return Piece::P;
            case Piece::n: return Piece::N;
            case Piece::b: return Piece::B;
            case Piece::r: return Piece::R;
            case Piece::q: return Piece::Q;
            case Piece::k: return Piece::K;
            case Piece::P: return Piece::p;
            case Piece::N: return Piece::n;
            case Piece::B: return Piece::b;
            case Piece::R: return Piece::r;
            case Piece::Q: return Piece::q;
            case Piece::K: return Piece::k;
            case Piece::none: return Piece::none;
        }
    }

    constexpr bool isWhite(Piece piece) { return getColor(piece) == Color::white; }
    constexpr bool isWhite(Color color) { return color == Color::white; }

    constexpr std::array<const char*, 64> square_to_coordinates = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    };

    constexpr int coordinateToIndex(const std::string& str)
    {
        for ( int i = 0; i < 64; ++i ) {
            if ( square_to_coordinates[i] == str ) {
                return i;
            }
        }

        return 65;
    }

    constexpr Color pieceColor(Piece piece)
    {
        if ( piece >= Piece::P && piece <= Piece::K ) {
            return Color::white;
        }
        else if ( piece >= Piece::p && piece <= Piece::k ) {
            return Color::black;
        }
        return Color::none;
    }

    constexpr Piece newCharToPiece(char c)
    {
        switch ( c ) {
            case 'P': return Piece::P; case 'N': return Piece::N;
            case 'B': return Piece::B; case 'R': return Piece::R;
            case 'Q': return Piece::Q; case 'K': return Piece::K;
            case 'p': return Piece::p; case 'n': return Piece::n;
            case 'b': return Piece::b; case 'r': return Piece::r;
            case 'q': return Piece::q; case 'k': return Piece::k;
            default: return Piece::none;
        }
    }

    constexpr char PieceToChar(Piece type)
    {
        switch ( type ) {
            case Piece::P: return 'P';
            case Piece::N: return 'N';
            case Piece::B: return 'B';
            case Piece::R: return 'R';
            case Piece::Q: return 'Q';
            case Piece::K: return 'K';
            case Piece::p: return 'p';
            case Piece::n: return 'n';
            case Piece::b: return 'b';
            case Piece::r: return 'r';
            case Piece::q: return 'q';
            case Piece::k: return 'k';
            default: return ' ';
        }
    }

    constexpr std::string_view PieceToUnicode(Piece type)
    {
        switch ( type ) {
            case Piece::P: return "♟︎";
            case Piece::N: return "♞";
            case Piece::B: return "♝";
            case Piece::R: return "♜";
            case Piece::Q: return "♛";
            case Piece::K: return "♚";
            case Piece::p: return "♙";
            case Piece::n: return "♘";
            case Piece::b: return "♗";
            case Piece::r: return "♖";
            case Piece::q: return "♕";
            case Piece::k: return "♔";
            default: return " ";
        }
    }

    constexpr Piece getPiece(PieceType type, Color color)
    {
        switch ( type ) {
            case PieceType::pawn:   return isWhite(color) ? Piece::P : Piece::p;
            case PieceType::knight: return isWhite(color) ? Piece::N : Piece::n;
            case PieceType::bishop: return isWhite(color) ? Piece::B : Piece::b;
            case PieceType::rook:   return isWhite(color) ? Piece::R : Piece::r;
            case PieceType::queen:  return isWhite(color) ? Piece::Q : Piece::q;
            case PieceType::king:   return isWhite(color) ? Piece::K : Piece::k;
            default:                      return Piece::none;
        }
    }

    constexpr PieceType getPieceType(Piece piece)
    {
        switch ( piece ) {
            case Piece::P: return PieceType::pawn;
            case Piece::N: return PieceType::knight;
            case Piece::B: return PieceType::bishop;
            case Piece::R: return PieceType::rook;
            case Piece::Q: return PieceType::queen;
            case Piece::K: return PieceType::king;
            case Piece::p: return PieceType::pawn;
            case Piece::n: return PieceType::knight;
            case Piece::b: return PieceType::bishop;
            case Piece::r: return PieceType::rook;
            case Piece::q: return PieceType::queen;
            case Piece::k: return PieceType::king;
            default:          return PieceType::none;
        }
    }

    constexpr Piece getPiece(char c)
    {
        switch ( c ) {
            case 'p': return Piece::p;
            case 'n': return Piece::n;
            case 'b': return Piece::b;
            case 'r': return Piece::r;
            case 'q': return Piece::q;
            case 'k': return Piece::k;
            case 'P': return Piece::P;
            case 'N': return Piece::N;
            case 'B': return Piece::B;
            case 'R': return Piece::R;
            case 'Q': return Piece::Q;
            case 'K': return Piece::K;
            case ' ': return Piece::none;
        }

        LOG_ERROR << "char: " << c << '\n';
        throw std::runtime_error("char to piece failed, char does not exist");
    }

    constexpr Color getColor(char c)
    {
        if ( isWhite(getPiece(c)) ) {
            return Color::white;
        }
        else {
            return Color::black;
        }
    }

    constexpr char getChar(Color color)
    {
        if ( isWhite(color) ) return 'w';
        else return 'b';
    }

    constexpr char getChar(Color color, PieceType type)
    {
        return PieceToChar(getPiece(type, color));
    }

}; // namespace type