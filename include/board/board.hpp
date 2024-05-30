#pragma once

#include "board.h"
#include "../zobrist.h"

template <PieceType type, Color color>
constexpr int Board::getIndex()
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
constexpr int Board::getIndex()
{
    return getIndex<utils::getPieceType(piece), utils::getColor(piece)>();
}

template <PieceType type, Color color>
constexpr uint64_t Board::getPieces() const
{
    constexpr int index = getIndex<type, color>();
    if constexpr ( index == 14 ) {
        return state->pieces[12] | state->pieces[13];
    }
    else {
        return state->pieces[index];
    }
}

template <Piece piece>
constexpr uint64_t Board::getPieces() const
{
    return getPieces<utils::getPieceType(piece), utils::getColor(piece)>();
}

// ================================
// Place pieces
// ================================

// IMPORTANT! from & to are assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr void Board::movePiece(uint64_t from, uint64_t to)
{
    constexpr Piece piece = utils::getPiece(type, color);
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex<type, color>();

    const uint64_t from_mask = single_bit_u64(from);
    const uint64_t to_mask = single_bit_u64(to);

    const uint64_t mask = from_mask | to_mask;

    state->pieces[piece_index] ^= mask;

    state->mailbox[from] = Piece::none;
    state->mailbox[to] = piece;

    state->pieces[occupancy_index] ^= mask;

    Zobrist::togglePiece(state->zobrist_hash, piece_index, from);
    Zobrist::togglePiece(state->zobrist_hash, piece_index, to);
}

// IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr void Board::removePiece(uint64_t square)
{
    constexpr int piece_index = getIndex<type, color>();
    constexpr int occ_index = getIndex<PieceType::none, color>();
    const uint64_t mask = ~single_bit_u64(square);

    state->pieces[piece_index] &= mask;
    state->pieces[occ_index] &= mask;

    state->mailbox[square] = Piece::none;

    Zobrist::togglePiece(state->zobrist_hash, piece_index, square);
}

// IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
template <PieceType type, Color color>
constexpr void Board::placePiece(uint64_t square)
{
    constexpr Piece piece = utils::getPiece(type, color);
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex<type, color>();
    const uint64_t mask = single_bit_u64(square);

    state->mailbox[square] = piece;
    state->pieces[piece_index] |= mask;

    state->pieces[occupancy_index] |= mask;

    Zobrist::togglePiece(state->zobrist_hash, piece_index, square);
}

template <Color color>
constexpr void Board::removePiece(Piece piece, uint64_t square)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = ~single_bit_u64(square);

    state->mailbox[square] = Piece::none;
    state->pieces[piece_index] &= mask;

    state->pieces[occupancy_index] &= mask;

    Zobrist::togglePiece(state->zobrist_hash, piece_index, square);
}

template <Color color>
constexpr void Board::placePiece(Piece piece, uint64_t square)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);
    const uint64_t mask = single_bit_u64(square);

    state->mailbox[square] = piece;
    state->pieces[piece_index] |= mask;

    state->pieces[occupancy_index] |= mask;

    Zobrist::togglePiece(state->zobrist_hash, piece_index, square);
}

template <Color color>
constexpr void Board::movePiece(Piece piece, uint64_t from, uint64_t to)
{
    constexpr int occupancy_index = getIndex<PieceType::none, color>();
    const int piece_index = getIndex(piece);

    const uint64_t from_mask = single_bit_u64(from);
    const uint64_t to_mask = single_bit_u64(to);

    const uint64_t mask = from_mask | to_mask;

    state->pieces[piece_index] ^= mask;

    state->mailbox[from] = Piece::none;
    state->mailbox[to] = piece;

    state->pieces[occupancy_index] ^= mask;

    Zobrist::togglePiece(state->zobrist_hash, piece_index, from);
    Zobrist::togglePiece(state->zobrist_hash, piece_index, to);
}


template <Color color>
void Board::storeState(const Move& move)
{
    MoveState new_state;

    const uint64_t from = move.getFrom();
    const uint64_t to = move.getTo();

    new_state.moving_piece = getPiece(from);
    new_state.captured_piece = getPiece(to);
    new_state.promotion_piece = move.getPromotionPiece<color>();

    new_state.ep_field = state->ep_field;
    new_state.zobrist_hash = state->zobrist_hash;

    new_state.castling_rights = state->castling_rights.raw;

    move_history.push(new_state);
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

    const uint64_t from = move.getFrom();
    const uint64_t to = move.getTo();

    const Piece moving_piece = move_history.top().moving_piece;

    if constexpr ( is_capture ) {
        constexpr int enemy_rook_k = (!is_white ? 7 : 63);
        constexpr int enemy_rook_q = (!is_white ? 0 : 56);

        if ( to == enemy_rook_k ) {
            removeCastleKs<enemy_color>();

            Zobrist::toggleCastlingKs<enemy_color>(state->zobrist_hash);
        }
        else if ( to == enemy_rook_q ) {
            removeCastleQs<enemy_color>();

            Zobrist::toggleCastlingQs<enemy_color>(state->zobrist_hash);
        }
    }

    if ( from == my_rook_k ) {
        removeCastleKs<my_color>();

        Zobrist::toggleCastlingKs<my_color>(state->zobrist_hash);
    }
    else if ( from == my_rook_q ) {
        removeCastleQs<my_color>();

        Zobrist::toggleCastlingQs<my_color>(state->zobrist_hash);
    }
    else if ( moving_piece == king ) {
        removeCastle<my_color>();

        Zobrist::toggleCastling<my_color>(state->zobrist_hash);
    }
}

// ================================
// make move / unmake move
// ================================
template <Color color>
void Board::move(const Move& move)
{
    storeState<color>(move);
    auto cur_state = move_history.top();

    constexpr Color my_color = color;
    constexpr Color enemy_color = utils::switchColor(color);

    const uint64_t move_to = move.getTo();
    const uint64_t move_from = move.getFrom();
    const Move::Flag move_flag = move.getFlag();

    const Piece moving_piece = cur_state.moving_piece;
    const Piece captured_piece = cur_state.captured_piece;

    constexpr auto pawn_push_function = (utils::isWhite(my_color) ? north : south);

    Zobrist::toggleBlackToMove(state->zobrist_hash);
    Zobrist::toggleEnPassant(state->zobrist_hash, state->ep_field);

    if ( move_flag == Move::Flag::pawn_push ) {
        movePiece<PieceType::pawn, my_color>(move_from, move_to);
        const uint64_t new_ep_field = pawn_push_function(1ULL << move_from);

        state->ep_field = new_ep_field;
        state->cur_color = enemy_color;

        return; // early exit because we set the ep field
    }

    else if ( move_flag == Move::Flag::quiet ) {
        movePiece<my_color>(moving_piece, move_from, move_to);
        tryToRemoveCastlingRights<my_color, false>(move);
    }

    else if ( move_flag == Move::Flag::castle_k ) {
        constexpr int rook_from = (utils::isWhite(my_color) ? 7 : 63);
        constexpr int rook_to = (utils::isWhite(my_color) ? 5 : 61);

        movePiece<PieceType::king, my_color>(move_from, move_to);
        movePiece<PieceType::rook, my_color>(rook_from, rook_to);

        removeCastle<my_color>();

        Zobrist::toggleCastlingKs<my_color>(state->zobrist_hash);
    }

    else if ( move_flag == Move::Flag::castle_q ) {
        constexpr int rook_from = (utils::isWhite(my_color) ? 0 : 56);
        constexpr int rook_to = (utils::isWhite(my_color) ? 3 : 59);

        movePiece<PieceType::king, my_color>(move_from, move_to);
        movePiece<PieceType::rook, my_color>(rook_from, rook_to);

        removeCastle<my_color>();

        Zobrist::toggleCastlingQs<my_color>(state->zobrist_hash);
    }

    else if ( move_flag == Move::Flag::capture ) {
        movePiece<color>(moving_piece, move_from, move_to);
        removePiece<enemy_color>(captured_piece, move_to);
        state->mailbox[move_to] = moving_piece;

        tryToRemoveCastlingRights<my_color, true>(move);
    }

    else if ( move_flag == Move::Flag::ep ) {
        constexpr int offset = (utils::isWhite(my_color) ? -8 : 8);
        const uint64_t enemy_square = move_to + offset;
        movePiece<PieceType::pawn, my_color>(move_from, move_to);
        removePiece<PieceType::pawn, enemy_color>(enemy_square);
    }

    else if ( move_flag == Move::Flag::promo_n || move_flag == Move::Flag::promo_b || move_flag == Move::Flag::promo_r || move_flag == Move::Flag::promo_q || move_flag == Move::Flag::promo_x_n || move_flag == Move::Flag::promo_x_b || move_flag == Move::Flag::promo_x_r || move_flag == Move::Flag::promo_x_q ) {
        if ( move.isCapture() ) {
            removePiece<enemy_color>(captured_piece, move_to);
            tryToRemoveCastlingRights<my_color, true>(move);
        }

        removePiece<PieceType::pawn, my_color>(move_from);
        placePiece<my_color>(cur_state.promotion_piece, move_to);
    }

    state->ep_field = 0ULL;
    state->cur_color = enemy_color;
}

template <Color color>
void Board::undo(const Move& move)
{
    if ( move_history.empty() ) {
        throw std::runtime_error("move history is empty\n");
    }

    constexpr bool is_white = utils::isWhite(color);
    constexpr Color my_color = color;
    constexpr Color enemy_color = utils::switchColor(color);

    MoveState last_state = move_history.top();
    move_history.pop();

    state->cur_color = enemy_color;
    state->ep_field = last_state.ep_field;
    state->castling_rights.raw = last_state.castling_rights;

    const uint64_t move_to = move.getTo();
    const uint64_t move_from = move.getFrom();
    const Move::Flag move_flag = move.getFlag();

    const Piece captured_piece = last_state.captured_piece;

    constexpr int ep_offset = (is_white ? -8 : 8);

    Zobrist::toggleBlackToMove(state->zobrist_hash);
    Zobrist::toggleEnPassant(state->zobrist_hash, state->ep_field);

    // undo normal moves
    switch ( move_flag ) {
        case Move::Flag::quiet:
        case Move::Flag::pawn_push:
        case Move::Flag::capture:
        case Move::Flag::ep: {
            movePiece<my_color>(getPiece(move_to), move_to, move_from);
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

        Zobrist::toggleCastling<my_color>(state->zobrist_hash);
        movePiece<PieceType::king, my_color>(move_to, move_from);
        return;
    }
    else if ( move.isEnpassant() ) {
        placePiece<PieceType::pawn, enemy_color>(move_to + ep_offset);
    }
    else if ( move.isPromotion() ) {
        const Piece promotion_piece = move.getPromotionPiece<color>();

        removePiece<my_color>(promotion_piece, move_to);
        placePiece<PieceType::pawn, my_color>(move_from);

        if ( move.isCapture() ) {
            placePiece<enemy_color>(captured_piece, move_to);
        }
    }
    else if ( move.isCapture() ) {
        placePiece<enemy_color>(captured_piece, move_to);
    }

    state->zobrist_hash = last_state.zobrist_hash;
}