#include "StateEncoding.h"
#include "Node.h"
#include "Piece.h"
#include "DeckConfig.h"

namespace StateEncoding {

std::string encodeState(const Node& node) {
    unsigned char data[12] = {0};
    int bitIdx = 0;

    // Encode board: 3 bits per square
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            Position pos(x, y);
            int val = 0; // empty
            for (const auto& p : node.m_piece) {
                if (p.mPosition == pos) {
                    if (p.mColor == Piece::Blue && p.mIsMaster) val = 1;
                    else if (p.mColor == Piece::Blue) val = 2;
                    else if (p.mColor == Piece::Red && p.mIsMaster) val = 3;
                    else val = 4;
                    break;
                }
            }
            // Pack 3 bits
            for (int b = 0; b < 3; b++) {
                int byteIdx = bitIdx / 8;
                int bitInByte = bitIdx % 8;
                if ((val >> b) & 1) {
                    data[byteIdx] |= (1 << bitInByte);
                }
                bitIdx++;
            }
        }
    }

    // Encode cards: 5 slots × 4 bits each
    for (int slot = 0; slot < 5; slot++) {
        int card = node.m_cards.card(slot);
        for (int b = 0; b < 4; b++) {
            int byteIdx = bitIdx / 8;
            int bitInByte = bitIdx % 8;
            if ((card >> b) & 1) {
                data[byteIdx] |= (1 << bitInByte);
            }
            bitIdx++;
        }
    }

    // Encode whose turn: 1 bit
    int turn = (node.m_tomove == Piece::Red) ? 1 : 0;
    if (turn) {
        data[11] |= (1 << (bitIdx % 8));
    }

    // Base64 encode
    static const char* b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    for (int i = 0; i < 12; i += 3) {
        unsigned b = (data[i] << 16) | ((i+1 < 12 ? data[i+1] : 0) << 8) | (i+2 < 12 ? data[i+2] : 0);
        result += b64chars[(b >> 18) & 63];
        result += b64chars[(b >> 12) & 63];
        if (i+1 < 12) result += b64chars[(b >> 6) & 63];
        if (i+2 < 12) result += b64chars[b & 63];
    }
    return result;
}

bool decodeState(const std::string& encoded, Node& node) {
    // Base64 decode
    static const std::string b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char data[12] = {0};
    int dataIdx = 0;

    for (size_t i = 0; i < encoded.length() && dataIdx < 12; i += 4) {
        unsigned b = 0;
        for (int j = 0; j < 4 && i+j < encoded.length(); j++) {
            size_t pos = b64chars.find(encoded[i+j]);
            if (pos == std::string::npos) return false;
            b = (b << 6) | pos;
        }
        if (i+1 < encoded.length()) data[dataIdx++] = (b >> 16) & 0xFF;
        if (i+2 < encoded.length()) data[dataIdx++] = (b >> 8) & 0xFF;
        if (i+3 < encoded.length()) data[dataIdx++] = b & 0xFF;
    }

    // Reset node
    node = Node();
    std::mt19937 rng(0);
    node.m_cards.deal(rng);

    int bitIdx = 0;

    // Decode board
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int val = 0;
            for (int b = 0; b < 3; b++) {
                int byteIdx = bitIdx / 8;
                int bitInByte = bitIdx % 8;
                if ((data[byteIdx] >> bitInByte) & 1) {
                    val |= (1 << b);
                }
                bitIdx++;
            }
            if (val > 0) {
                Position pos(x, y);
                bool isMaster = (val == 1 || val == 3);
                Piece::Color color = (val == 1 || val == 2) ? Piece::Blue : Piece::Red;
                node.addPiece(Piece(pos, color, isMaster));
            }
        }
    }

    // Decode cards
    for (int slot = 0; slot < 5; slot++) {
        int card = 0;
        for (int b = 0; b < 4; b++) {
            int byteIdx = bitIdx / 8;
            int bitInByte = bitIdx % 8;
            if ((data[byteIdx] >> bitInByte) & 1) {
                card |= (1 << b);
            }
            bitIdx++;
        }
        node.m_cards.set(slot, static_cast<CurrentCard::CardType>(card));
    }

    // Decode whose turn
    int byteIdx = bitIdx / 8;
    int bitInByte = bitIdx % 8;
    int turn = (data[byteIdx] >> bitInByte) & 1;
    node.m_tomove = (turn == 1) ? Piece::Red : Piece::Blue;

    return true;
}

} // namespace StateEncoding
