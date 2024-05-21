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
constexpr inline uint64_t Board::getPieces() const
{
    return getPieces<utils::getPieceType(piece), utils::getColor(piece)>();
}

// ================================
// Place pieces
// ================================

// IMPORTANT! from & to are assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr inline void Board::movePiece(uint64_t from, uint64_t to)
{
    constexpr Piece piece = utils::getPiece(type, color);
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex<type, color>();

    const uint64_t from_mask = single_bit_u64(from);
    const uint64_t to_mask = single_bit_u64(to);

    const uint64_t mask = from_mask | to_mask;

    pieces[piece_index] ^= mask;

    mailbox[from] = Piece::none;
    mailbox[to] = piece;

    pieces[occupancy_index] ^= mask;
}

// IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr inline void Board::removePiece(uint64_t square)
{
    constexpr int piece_index = getIndex<type, color>();
    constexpr int occ_index = getIndex<PieceType::none, color>();
    const uint64_t mask = ~single_bit_u64(square);

    pieces[piece_index] &= mask;
    pieces[occ_index] &= mask;

    mailbox[square] = Piece::none;
}

// IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr inline void Board::placePiece(uint64_t square)
{
    constexpr Piece piece = utils::getPiece(type, color);
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex<type, color>();
    const uint64_t mask = single_bit_u64(square);

    mailbox[square] = piece;
    pieces[piece_index] |= mask;

    pieces[occupancy_index] |= mask;
}

template <Color color>
inline void Board::remove_piece(Piece piece, int square)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = ~single_bit_u64(square);

    mailbox[square] = Piece::none;
    pieces[piece_index] &= mask;

    pieces[occupancy_index] &= mask;
}

template <Color color>
inline void Board::place_piece(Piece piece, int square)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = single_bit_u64(square);

    mailbox[square] = piece;
    pieces[piece_index] |= mask;

    pieces[occupancy_index] |= mask;
}

template <Color color>
inline void Board::move_piece(Piece piece, int from, int to)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);

    const uint64_t from_mask = single_bit_u64(from);
    const uint64_t to_mask = single_bit_u64(to);

    const uint64_t mask = from_mask | to_mask;

    pieces[piece_index] ^= mask;

    mailbox[from] = Piece::none;
    mailbox[to] = piece;

    pieces[occupancy_index] ^= mask;
}

// ================================
// Remove Castling
// ================================
// hacky way to disable castling rights;
template <Color color, bool is_capture>
inline void Board::tryToRemoveCastlingRights(const Move& move)
{
    constexpr Color my_color = color;
    constexpr Color enemy_color = utils::switchColor(my_color);

    if constexpr ( is_capture ) {
        if ( !canCastle() ) {
            return;
        }
    }
    else {
        if ( !canCastle<color>() ) {
            return;
        }
    }

    constexpr bool is_white = utils::isWhite(my_color);
    constexpr int my_rook_k = (is_white ? 7 : 63);
    constexpr int my_rook_q = (is_white ? 0 : 56);

    constexpr Piece king = utils::getPiece(PieceType::king, my_color);

    const Piece moving_piece = move_history.top().moving_piece;
    const int from = move.getFrom();
    const int to = move.getTo();

    if constexpr ( is_capture ) {
        constexpr int enemy_rook_k = (!is_white ? 7 : 63);
        constexpr int enemy_rook_q = (!is_white ? 0 : 56);

        if ( to == enemy_rook_k ) {
            removeCastleKs<enemy_color>();
        }
        else if ( to == enemy_rook_q ) {
            removeCastleQs<enemy_color>();
        }
    }

    if ( from == my_rook_k ) {
        removeCastleKs<my_color>();
    }
    else if ( from == my_rook_q ) {
        removeCastleQs<my_color>();
    }
    else if ( moving_piece == king ) {
        removeCastle<my_color>();
    }
}

// ================================
// make move / unmake move
// ================================
template <Color color>
void Board::move(const Move& move)
{
    DEBUG_START;

    storeState(move);
    auto state = move_history.top();

    constexpr Color my_color = color;
    constexpr Color enemy_color = utils::switchColor(color);

    const uint64_t move_to = move.getTo();
    const uint64_t move_from = move.getFrom();
    const Move::Flag move_flag = move.getFlag();

    const Piece moving_piece = state.moving_piece;
    const Piece captured_piece = state.captured_piece;

    constexpr auto pawn_push_function = (utils::isWhite(my_color) ? north : south);

    move_piece<color>(moving_piece, move_from, move_to);
    switch ( move_flag ) {
        case Move::Flag::quiet: {
            tryToRemoveCastlingRights<my_color, false>(move);
        } break;
        case Move::Flag::pawn_push: {
            const uint64_t new_ep_field = pawn_push_function(1ULL << move_from);
            ep_field = new_ep_field;

            cur_color = enemy_color;
            return; // early exit because we set the ep field
        } break;
        case Move::Flag::castle_k: {
            constexpr int rook_from = (utils::isWhite(my_color) ? 7 : 63);
            constexpr int rook_to = (utils::isWhite(my_color) ? 5 : 61);

            movePiece<PieceType::rook, my_color>(rook_from, rook_to);
            removeCastle<my_color>();
        } break;
        case Move::Flag::castle_q: {
            constexpr int rook_from = (utils::isWhite(my_color) ? 0 : 56);
            constexpr int rook_to = (utils::isWhite(my_color) ? 3 : 59);

            movePiece<PieceType::rook, my_color>(rook_from, rook_to);
            removeCastle<my_color>();
        } break;
        case Move::Flag::capture: {
            remove_piece<enemy_color>(captured_piece, move_to);
            mailbox[move_to] = moving_piece;

            tryToRemoveCastlingRights<my_color, true>(move);
        } break;
        case Move::Flag::ep: {
            constexpr int offset = (utils::isWhite(my_color) ? -8 : 8);
            removePiece<PieceType::pawn, enemy_color>(move_to + offset);
        } break;
        case Move::Flag::promo_n:
        case Move::Flag::promo_b:
        case Move::Flag::promo_r:
        case Move::Flag::promo_q:
        case Move::Flag::promo_x_n:
        case Move::Flag::promo_x_b:
        case Move::Flag::promo_x_r:
        case Move::Flag::promo_x_q: {
            state.promotion_piece = utils::getPiece(move.getPromotionPieceType(), my_color);
            move_history.top().promotion_piece = state.promotion_piece;

            if ( move.isCapture() ) {
                remove_piece<enemy_color>(captured_piece, move_to);
                tryToRemoveCastlingRights<my_color, true>(move);
            }

            //remove_piece<my_color>(moving_piece, move_to); // remove the pawn we moved earlier
            removePiece<PieceType::pawn, my_color>(move_to);
            place_piece<my_color>(state.promotion_piece, move_to);
        } break;
        default: break;
    }

    ep_field = 0ULL;
    cur_color = enemy_color;

    DEBUG_END;
}

template <Color color>
void Board::undo(const Move& move)
{
    DEBUG_START;

    if ( move_history.empty() ) {
        LOG_ERROR << "MOVE HISTORY IS EMPTY!\n";
        throw std::runtime_error("mh is empty\n");
    }

    constexpr bool is_white = utils::isWhite(color);
    constexpr Color my_color = color;
    constexpr Color enemy_color = utils::switchColor(color);

    MoveState last_state = move_history.top();
    move_history.pop();

    cur_color = enemy_color;
    ep_field = last_state.ep_field_before;
    castling_rights.raw = last_state.castling_rights;

    const uint64_t move_to = move.getTo();
    const uint64_t move_from = move.getFrom();
    const Move::Flag move_flag = move.getFlag();

    const Piece moving_piece = last_state.moving_piece;
    const Piece captured_piece = last_state.captured_piece;
    const Piece promotion_piece = last_state.promotion_piece;

    constexpr int ep_offset = (is_white ? -8 : 8);

    // undo normal moves
    switch ( move_flag ) {
        case Move::Flag::quiet:
        case Move::Flag::pawn_push:
        case Move::Flag::castle_k:
        case Move::Flag::castle_q:
        case Move::Flag::capture:
        case Move::Flag::ep: {
            move_piece<my_color>(moving_piece, move_to, move_from);
        } break;
        default: break;
    }

    if ( move.isCastle() ) {
        if ( move_flag == Move::Flag::castle_k ) {
            constexpr int rook_from = (utils::isWhite(my_color) ? 7 : 63);
            constexpr int rook_to = (utils::isWhite(my_color) ? 5 : 61);

            movePiece<PieceType::rook, my_color>(rook_to, rook_from);
        }
        else if ( move_flag == Move::Flag::castle_q ) {
            constexpr int rook_from = (utils::isWhite(my_color) ? 0 : 56);
            constexpr int rook_to = (utils::isWhite(my_color) ? 3 : 59);

            movePiece<PieceType::rook, my_color>(rook_to, rook_from);
        }

        return;
    }

    if ( move.isEnpassant() ) {
        placePiece<PieceType::pawn, enemy_color>(move_to + ep_offset);
    }
    else if ( move.isPromotion() ) {
        remove_piece<my_color>(promotion_piece, move_to);
        placePiece<PieceType::pawn, my_color>(move_from);

        if ( move.isCapture() ) {
            place_piece<enemy_color>(captured_piece, move_to);
        }
    }
    else if ( move.isCapture() ) {
        place_piece<enemy_color>(captured_piece, move_to);
    }

    DEBUG_END;
}