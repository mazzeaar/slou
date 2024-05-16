#pragma once

#include "board.h"

template <PieceType type, Color color>
constexpr inline int Board::getIndex()
{
    if constexpr ( type == PieceType::none ) {
        if constexpr ( color == Color::none ) {
            return 14; // occupancy
        }
        else if constexpr ( color == Color::white ) {
            return 12;  // white pieces
        }
        else {
            return 13;  // black pieces
        }
    }
    else {
        constexpr Piece piece = utils::getPiece(type, color);
        return static_cast<int>(piece);
    }
}

template <Piece piece>
constexpr inline int Board::getIndex()
{
    return getIndex<utils::getPieceType(piece), utils::getColor(piece)>();
}

template <PieceType type, Color color>
constexpr inline uint64_t Board::getPieces() const
{
    constexpr int index = getIndex<type, color>();
    if constexpr ( index == 14 ) {
        return pieces[12] | pieces[13];
    }
    else {
        return pieces[index];
    }
}

template <Piece piece>
constexpr inline uint64_t Board::getPieces() const { return getPieces<utils::getPieceType(piece), utils::getColor(piece)>(); }

// IMPORTANT! from & to are assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr inline void Board::movePiece(uint64_t from, uint64_t to)
{
    constexpr int piece_index = getIndex<type, color>();
    constexpr int occ_index = getIndex<type, Color::none>();
    const uint64_t move = (1ULL << from) | (1ULL << to);

    pieces[piece_index] ^= move;
    pieces[occ_index] ^= move;

    mailbox[from] = Piece::none;
    mailbox[to] = utils::getPiece(type, color);
}

// IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr inline void Board::removePiece(uint64_t square)
{
    constexpr int piece_index = getIndex<type, color>();
    constexpr int occ_index = getIndex<type, Color::none>();
    const uint64_t move = (1ULL << square);

    pieces[piece_index] ^= move;
    pieces[occ_index] ^= move;

    mailbox[square] = Piece::none;
}

// ================================
// Place pieces
// ================================

// IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr inline void Board::placePiece(uint64_t square)
{
    constexpr int piece_index = getIndex<type, color>();
    constexpr int occ_index = getIndex<type, Color::none>();
    const uint64_t move = (1ULL << square);
    pieces[piece_index] ^= move;
    pieces[occ_index] ^= move;

    mailbox[square] = utils::getPiece(type, color);
}

template <Color color>
inline void Board::remove_piece(Piece piece, int square)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = ~single_bit_u64(square);

    pieces[piece_index] &= mask;

    mailbox[square] = Piece::none;

    pieces[occupancy_index] &= mask;
}

template <Color color>
inline void Board::place_piece(Piece piece, int square)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = single_bit_u64(square);

    pieces[piece_index] |= mask;

    mailbox[square] = piece;

    pieces[occupancy_index] |= mask;
}

template <Color color>
inline void Board::move_piece(Piece piece, int from, int to)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = (single_bit_u64(from) | single_bit_u64(to));

    pieces[piece_index] ^= mask;

    mailbox[from] = Piece::none;
    mailbox[to] = piece;

    pieces[occupancy_index] ^= mask;
}

// ================================
// make move / unmake move
// ================================

template <Color color>
void Board::move(const Move& move)
{
    storeState(move);
    auto state = move_history.top();

    const uint64_t move_to = move.getTo();
    const uint64_t move_from = move.getFrom();
    const Move::Flag move_flag = move.getFlag();

    const Piece moving_piece = state.moving_piece;
    const Piece captured_piece = state.captured_piece;

    constexpr Piece castling_rook = utils::getPiece(PieceType::rook, color);
    constexpr Piece ep_capture_piece = utils::getPiece(PieceType::pawn, utils::switchColor(color));
    constexpr auto pawn_push_function = (utils::isWhite(color) ? north : south);

    move_piece<color>(moving_piece, move_from, move_to);

    switch ( move_flag ) {
        case Move::Flag::quiet: {
            tryToRemoveCastlingRights(move);
        } break;
        case Move::Flag::pawn_push: {
            const uint64_t new_ep_field = pawn_push_function(1ULL << move_from);
            updateEpField(new_ep_field);

            cur_color = utils::switchColor(cur_color);
            return; // early exit because we set the ep field
        } break;
        case Move::Flag::castle_k: {
            constexpr int rook_from = (utils::isWhite(color) ? 7 : 63);
            constexpr int rook_to = (utils::isWhite(color) ? 5 : 61);

            move_piece<color>(castling_rook, rook_from, rook_to);
            removeCastle(color);
        } break;
        case Move::Flag::castle_q: {
            constexpr int rook_from = (utils::isWhite(color) ? 0 : 56);
            constexpr int rook_to = (utils::isWhite(color) ? 3 : 59);

            move_piece<color>(castling_rook, rook_from, rook_to);
            removeCastle(color);
        } break;
        case Move::Flag::capture: {
            remove_piece<color>(captured_piece, move_to);
            tryToRemoveCastlingRights(move);
        } break;
        case Move::Flag::ep: {
            constexpr int offset = (utils::isWhite(color) ? -8 : 8);
            remove_piece<color>(ep_capture_piece, move_to + offset);
        } break;
        case Move::Flag::promo_n:
        case Move::Flag::promo_b:
        case Move::Flag::promo_r:
        case Move::Flag::promo_q:
        case Move::Flag::promo_x_n:
        case Move::Flag::promo_x_b:
        case Move::Flag::promo_x_r:
        case Move::Flag::promo_x_q: {
            state.promotion_piece = utils::getPiece(move.getPromotionPieceType(), cur_color);
            move_history.top().promotion_piece = state.promotion_piece;

            if ( move.isCapture() ) {
                remove_piece<color>(captured_piece, move_to);
                tryToRemoveCastlingRights(move);
            }

            remove_piece<color>(moving_piece, move_to); // remove the pawn we moved earlier
            place_piece<color>(state.promotion_piece, move_to);
        } break;
        default: break;
    }

    updateEpField(0ULL);
    cur_color = utils::switchColor(cur_color);
}

template <Color color>
void Board::undo(const Move& move)
{
    if ( move_history.empty() ) {
        LOG_ERROR << "MOVE HISTORY IS EMPTY!\n";
        throw std::runtime_error("mh is empty\n");
    }

    // restoreState();
    // return;

    MoveState last_state = move_history.top();
    move_history.pop();

    cur_color = utils::switchColor(cur_color);
    updateEpField(last_state.ep_field_before);
    castling_rights.raw = last_state.castling_rights;

    const uint64_t move_to = move.getTo();
    const uint64_t move_from = move.getFrom();
    const Move::Flag move_flag = move.getFlag();

    const Piece moving_piece = last_state.moving_piece;
    const Piece captured_piece = last_state.captured_piece;
    const Piece promotion_piece = last_state.promotion_piece;

    constexpr Piece castling_rook = utils::getPiece(PieceType::rook, color);
    constexpr Piece ep_capture_piece = utils::getPiece(PieceType::pawn, utils::switchColor(color));
    constexpr int ep_offset = (utils::isWhite(color) ? -8 : 8);

    // undo normal moves
    switch ( move_flag ) {
        case Move::Flag::quiet:
        case Move::Flag::pawn_push:
        case Move::Flag::castle_k:
        case Move::Flag::castle_q:
        case Move::Flag::capture:
        case Move::Flag::ep: {
            move_piece<color>(moving_piece, move_to, move_from);
        } break;
        default: break;
    }

    if ( move.isCastle() ) {
        if ( move_flag == Move::Flag::castle_k ) {
            constexpr int rook_from = (utils::isWhite(color) ? 7 : 63);
            constexpr int rook_to = (utils::isWhite(color) ? 5 : 61);

            move_piece<color>(castling_rook, rook_to, rook_from);
        }
        else if ( move_flag == Move::Flag::castle_q ) {
            constexpr int rook_from = (utils::isWhite(color) ? 0 : 56);
            constexpr int rook_to = (utils::isWhite(color) ? 3 : 59);

            move_piece<color>(castling_rook, rook_to, rook_from);
        }

        return;
    }

    if ( move.isEnpassant() ) {
        place_piece<color>(ep_capture_piece, move_to + ep_offset);
        return;
    }

    if ( move.isCapture() ) {
        place_piece<color>(captured_piece, move_to);
    }

    if ( move.isPromotion() ) {
        remove_piece<color>(promotion_piece, move_to);
        place_piece<color>(moving_piece, move_from);
    }
}