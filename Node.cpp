//
// Created by Jan de Kort on 2019-08-18.
//

#include "Node.h"
#include "NodeVisitor.h"
#include "PieceVisitor.h"
#include "DeckConfig.h"


void Node::addPiece(Piece a_piece) {
    m_piece.push_back(a_piece);
}

void Node::accept(NodeVisitor &n)
{
    n.visit(*this);
}

const Piece* Node::pieceAt(Position aPos) const {
    for (const auto& p : m_piece)
        if (p.mPosition == aPos) return &p;
    return nullptr;
}

Piece* Node::pieceAt(Position aPos) {
    for (auto& p : m_piece)
        if (p.mPosition == aPos) return &p;
    return nullptr;
}

Position Node::temple(Piece::Color aColor) {
    // Red starts on the top row (y=0), Blue on the bottom row (y=4).
    return aColor == Piece::Red ? Position(2, 0) : Position(2, 4);
}

bool Node::isLoss() const {
    bool haveMaster = false;
    Position myTemple = temple(m_tomove);

    for (const auto& p : m_piece) {
        if (!p.mIsMaster) continue;
        if (p.mColor == m_tomove)
            haveMaster = true;                       // Way of the Stone check
        else if (p.mPosition == myTemple)
            return true;                             // Way of the Stream
    }
    return !haveMaster;
}

std::vector<Ply> Node::legalMoves() const {

    std::vector<Ply> result;

    int firstSlot = Cards::firstSlot(m_tomove);
    // Cards are written from the owner's perspective; Blue (bottom) applies
    // them as-is, Red (top) rotates them 180 degrees.
    int orientation = (m_tomove == Piece::Blue) ? 1 : -1;

    for (int slot = firstSlot; slot <= firstSlot + 1; slot++) {
        for (const auto& piece : m_piece) {
            if (piece.mColor != m_tomove) continue;
            for (const auto& move : CurrentCard::movesFor(m_cards.card(slot))) {
                Position to = piece.mPosition
                    + Position(move.mShiftX * orientation, move.mShiftY * orientation);
                if (!to.isOnBoard()) continue;
                const Piece* occupant = pieceAt(to);
                if (occupant && occupant->mColor == m_tomove) continue;

                Ply ply;
                ply.mCardSlot = slot;
                ply.mIsPass = false;
                ply.mFrom = piece.mPosition;
                ply.mTo = to;
                result.push_back(ply);
            }
        }
    }

    if (result.empty()) {
        // No piece can move: the player must still exchange one of his cards.
        for (int slot = firstSlot; slot <= firstSlot + 1; slot++) {
            Ply ply;
            ply.mCardSlot = slot;
            ply.mIsPass = true;
            result.push_back(ply);
        }
    }

    return result;
}

Node Node::applyPly(const Ply& aPly) const {

    Node child(*this);

    if (!aPly.mIsPass) {
        // Capture: remove an enemy piece on the destination square.
        for (auto it = child.m_piece.begin(); it != child.m_piece.end(); ++it) {
            if (it->mPosition == aPly.mTo) {
                child.m_piece.erase(it);
                break;
            }
        }

        Piece* mover = child.pieceAt(aPly.mFrom);
        if (!mover || mover->mColor != m_tomove)
            throw std::logic_error("applyPly: no own piece on the from-square");

        Move shift(aPly.mTo.mX - aPly.mFrom.mX, aPly.mTo.mY - aPly.mFrom.mY);
        mover->accept(shift);
    }

    child.m_cards.play(aPly.mCardSlot);
    child.m_tomove = Piece::other(m_tomove);
    child.m_score = 0.0;

    return child;
}

std::ostream& operator<<(std::ostream &out, const Node& lf) {

    char b[5][5]; // [x][y]
    for (int x = 0; x < 5; x++)
        for (int y = 0; y < 5; y++)
            b[x][y] = ' ';

    for (const auto& p : lf.m_piece)
        b[p.mPosition.mX][p.mPosition.mY] = p.show();

    for (int y = 0; y < 5; y++) {           // y=0 (Red side) printed on top
        out << (5 - y) << ' ';
        for (int x = 0; x < 5; x++)
            out << '[' << b[x][y] << ']';
        out << std::endl;
    }
    out << "   a  b  c  d  e" << std::endl;

    return out;
}
