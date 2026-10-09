#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

struct Round {
    char type;
    size_t width;
    vector<size_t> order;
};

string ask(const string& message) {
    cout << message;
    string answer;

    if (!getline(cin, answer))
        throw runtime_error("Input ended.");

    return answer;
}

size_t askNumber(const string& message,
    size_t minimum, size_t maximum) {
    while (true) {
        istringstream in(ask(message));
        size_t value;
        string extra;

        if ((in >> value) && !(in >> extra) &&
            value >= minimum && value <= maximum)
            return value;

        cout << "Enter a number from "
            << minimum << " to " << maximum << ".\n";
    }
}

vector<size_t> askOrder(size_t count) {
    while (true) {
        string line = ask("Permutation (e.g. 2 1 3): ");
        istringstream in(line);

        vector<size_t> order;
        vector<bool> used(count, false);

        size_t value;
        bool valid = true;

        while (in >> value) {
            if (value < 1 || value > count ||
                used[value - 1]) {
                valid = false;
                break;
            }

            used[value - 1] = true;
            order.push_back(value - 1);
        }

        if (valid && in.eof() && order.size() == count)
            return order;

        cout << "Enter each number from 1 to "
            << count << " exactly once.\n";
    }
}

bool letter(char c) {
    return (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

// Vigenere encryption and decryption
string vigenere(const string& text,
    const string& key, bool encrypt) {
    string result = text;
    size_t keyPos = 0;

    for (char& c : result) {
        if (!letter(c))
            continue;

        char base = (c >= 'a' && c <= 'z') ? 'a' : 'A';
        char k = key[keyPos % key.size()];

        int shift = (k >= 'a' && k <= 'z')
            ? k - 'a'
            : k - 'A';

        int value = c - base;

        c = static_cast<char>(
            base + (value +
                (encrypt ? shift : 26 - shift)) % 26
            );

        ++keyPos;
    }

    return result;
}

// Determine positions for row/column permutations
vector<size_t> positions(size_t length,
    const Round& round) {
    size_t rows = 1 + (length - 1) / round.width;

    vector<size_t> result;
    result.reserve(length);

    if (round.type == 'R') {
        for (size_t r : round.order) {
            for (size_t c = 0; c < round.width; ++c) {
                size_t pos = r * round.width + c;

                if (pos < length)
                    result.push_back(pos);
            }
        }
    }
    else {
        for (size_t c : round.order) {
            for (size_t r = 0; r < rows; ++r) {
                size_t pos = r * round.width + c;

                if (pos < length)
                    result.push_back(pos);
            }
        }
    }

    return result;
}

// Apply or reverse a permutation
string permute(const string& text,
    const Round& round, bool undo) {
    vector<size_t> index =
        positions(text.size(), round);

    string result(text.size(), ' ');

    for (size_t i = 0; i < text.size(); ++i) {
        if (undo)
            result[index[i]] = text[i];
        else
            result[i] = text[index[i]];
    }

    return result;
}

int main() {
    try {
        string mode;

        do {
            mode = ask(
                "Choose E (encrypt) or D (decrypt): "
            );
        } while (mode != "E" && mode != "e" &&
            mode != "D" && mode != "d");

        bool encrypt = mode == "E" || mode == "e";

        string text = ask(
            encrypt ? "Plaintext: " : "Ciphertext: "
        );

        if (text.empty()) {
            cout << "The input cannot be empty.\n";
            return 1;
        }

        string key;

        while (true) {
            key = ask("Secret key (letters only): ");

            if (!key.empty()) {
                bool valid = true;

                for (char c : key)
                    if (!letter(c))
                        valid = false;

                if (valid)
                    break;
            }

            cout << "The key must contain only A-Z letters.\n";
        }

        size_t count = askNumber(
            "Number of permutation rounds (1-20): ",
            1, 20
        );

        vector<Round> rounds;

        for (size_t i = 0; i < count; ++i) {
            cout << "\nRound " << i + 1
                << " (as applied during ENCRYPTION)\n";

            string kind;

            do {
                kind = ask(
                    "R = rearrange rows, "
                    "C = rearrange columns: "
                );
            } while (kind != "R" && kind != "r" &&
                kind != "C" && kind != "c");

            char type = (kind == "R" || kind == "r")
                ? 'R' : 'C';

            size_t width = askNumber(
                "Number of columns in grid: ",
                1, text.size()
            );

            size_t rows =
                1 + (text.size() - 1) / width;

            size_t required =
                (type == 'R') ? rows : width;

            cout << "Grid: " << rows
                << " row(s), " << width
                << " column(s).\n";

            cout << "Enter " << required
                << " distinct number(s) from 1 to "
                << required
                << " in the desired order.\n";

            rounds.push_back({
                type, width, askOrder(required)
                });
        }

        cout << "\n--- RESULTS ---\n";

        if (encrypt) {
            text = vigenere(text, key, true);

            cout << "After Vigenere substitution: "
                << text << '\n';

            for (size_t i = 0; i < rounds.size(); ++i) {
                text = permute(text, rounds[i], false);

                cout << "After round " << i + 1
                    << ": " << text << '\n';
            }

            cout << "\nFINAL CIPHERTEXT "
                << "(copy the next line exactly):\n"
                << text << '\n';
        }
        else {
            for (size_t i = rounds.size(); i > 0; --i) {
                text = permute(text, rounds[i - 1], true);

                cout << "After undoing round "
                    << i << ": " << text << '\n';
            }

            text = vigenere(text, key, false);

            cout << "After Vigenere decryption: "
                << text << '\n';

            cout << "\nRECOVERED PLAINTEXT:\n"
                << text << '\n';
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}