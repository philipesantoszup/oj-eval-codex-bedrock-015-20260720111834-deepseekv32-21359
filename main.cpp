#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

const string DATA_FILE = "storage.dat";

class FileStorage {
private:
    fstream file;
    
    void writeEntry(const string& key, int value, bool active) {
        // Write key (65 bytes, null-terminated)
        char keyBuf[65];
        strncpy(keyBuf, key.c_str(), 64);
        keyBuf[64] = '\0';
        file.write(keyBuf, 65);
        
        // Write value (4 bytes)
        file.write(reinterpret_cast<const char*>(&value), sizeof(int));
        
        // Write active flag (1 byte)
        file.write(reinterpret_cast<const char*>(&active), sizeof(bool));
    }
    
    bool readEntry(string& key, int& value, bool& active) {
        // Read key
        char keyBuf[65];
        if (!file.read(keyBuf, 65)) return false;
        key = keyBuf;
        
        // Read value
        if (!file.read(reinterpret_cast<char*>(&value), sizeof(int))) return false;
        
        // Read active flag
        if (!file.read(reinterpret_cast<char*>(&active), sizeof(bool))) return false;
        
        return true;
    }
    
public:
    FileStorage() {
        // Open or create file
        file.open(DATA_FILE, ios::in | ios::out | ios::binary);
        if (!file) {
            // Create file if it doesn't exist
            file.open(DATA_FILE, ios::out | ios::binary);
            file.close();
            file.open(DATA_FILE, ios::in | ios::out | ios::binary);
        }
    }
    
    ~FileStorage() {
        if (file.is_open()) {
            file.close();
        }
    }
    
    void insert(const string& key, int value) {
        // Check if entry already exists
        file.seekg(0, ios::beg);
        string readKey;
        int readValue;
        bool readActive;
        
        while (readEntry(readKey, readValue, readActive)) {
            if (readActive && readKey == key && readValue == value) {
                return; // Entry already exists
            }
        }
        
        // Write new entry
        file.clear();
        file.seekp(0, ios::end);
        writeEntry(key, value, true);
        file.flush();
    }
    
    void remove(const string& key, int value) {
        file.seekg(0, ios::beg);
        streampos pos = 0;
        string readKey;
        int readValue;
        bool readActive;
        
        while (readEntry(readKey, readValue, readActive)) {
            if (readActive && readKey == key && readValue == value) {
                // Mark as deleted
                file.seekp(pos);
                writeEntry(key, value, false);
                file.flush();
                return;
            }
            pos = file.tellg();
        }
    }
    
    vector<int> find(const string& key) {
        vector<int> results;
        file.seekg(0, ios::beg);
        string readKey;
        int readValue;
        bool readActive;
        
        while (readEntry(readKey, readValue, readActive)) {
            if (readActive && readKey == key) {
                results.push_back(readValue);
            }
        }
        
        sort(results.begin(), results.end());
        return results;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;
    
    FileStorage storage;
    
    for (int i = 0; i < n; i++) {
        string command;
        cin >> command;
        
        if (command == "insert") {
            string key;
            int value;
            cin >> key >> value;
            storage.insert(key, value);
        } else if (command == "delete") {
            string key;
            int value;
            cin >> key >> value;
            storage.remove(key, value);
        } else if (command == "find") {
            string key;
            cin >> key;
            vector<int> results = storage.find(key);
            
            if (results.empty()) {
                cout << "null\n";
            } else {
                for (size_t j = 0; j < results.size(); j++) {
                    if (j > 0) cout << " ";
                    cout << results[j];
                }
                cout << "\n";
            }
        }
    }
    
    return 0;
}
