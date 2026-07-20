#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

const int NUM_BUCKETS = 20;
const string DATA_FILE_PREFIX = "bucket_";

// Simple deterministic hash
size_t hash_string(const string& str) {
    size_t h = 0;
    for (char c : str) {
        h = h * 31 + static_cast<unsigned char>(c);
    }
    return h;
}

int get_bucket(const string& key) {
    return hash_string(key) % NUM_BUCKETS;
}

string get_bucket_filename(int bucket) {
    return DATA_FILE_PREFIX + to_string(bucket) + ".dat";
}

class FileStorage {
private:
    vector<fstream> bucketFiles;
    
    void writeEntry(fstream& file, const string& key, int value, bool active) {
        // Write key (65 bytes, null-terminated)
        char keyBuf[65];
        strncpy(keyBuf, key.c_str(), 64);
        keyBuf[64] = '\0';
        file.write(keyBuf, 65);
        
        // Write value (4 bytes)
        file.write(reinterpret_cast<const char*>(&value), sizeof(int));
        
        // Write active flag (1 byte)
        char activeByte = active ? 1 : 0;
        file.write(&activeByte, 1);
    }
    
    bool readEntry(fstream& file, string& key, int& value, bool& active) {
        // Read key
        char keyBuf[65];
        if (!file.read(keyBuf, 65)) return false;
        key = keyBuf;
        
        // Read value
        if (!file.read(reinterpret_cast<char*>(&value), sizeof(int))) return false;
        
        // Read active flag
        char activeByte;
        if (!file.read(&activeByte, 1)) return false;
        active = (activeByte != 0);
        
        return true;
    }
    
public:
    FileStorage() {
        bucketFiles.resize(NUM_BUCKETS);
    }
    
    ~FileStorage() {
        for (auto& file : bucketFiles) {
            if (file.is_open()) {
                file.close();
            }
        }
    }
    
    void ensureOpen(int bucket) {
        if (!bucketFiles[bucket].is_open()) {
            bucketFiles[bucket].open(get_bucket_filename(bucket), 
                                   ios::in | ios::out | ios::binary);
            if (!bucketFiles[bucket]) {
                // Create file if it doesn't exist
                bucketFiles[bucket].open(get_bucket_filename(bucket), 
                                       ios::out | ios::binary);
                bucketFiles[bucket].close();
                bucketFiles[bucket].open(get_bucket_filename(bucket), 
                                       ios::in | ios::out | ios::binary);
            }
        }
    }
    
    void insert(const string& key, int value) {
        int bucket = get_bucket(key);
        ensureOpen(bucket);
        fstream& file = bucketFiles[bucket];
        
        // Check if entry already exists
        file.seekg(0, ios::beg);
        string readKey;
        int readValue;
        bool readActive;
        
        while (readEntry(file, readKey, readValue, readActive)) {
            if (readActive && readKey == key && readValue == value) {
                return; // Entry already exists
            }
        }
        
        // Write new entry
        file.clear();
        file.seekp(0, ios::end);
        writeEntry(file, key, value, true);
        file.flush();
    }
    
    void remove(const string& key, int value) {
        int bucket = get_bucket(key);
        ensureOpen(bucket);
        fstream& file = bucketFiles[bucket];
        
        file.seekg(0, ios::beg);
        streampos pos = 0;
        string readKey;
        int readValue;
        bool readActive;
        
        while (readEntry(file, readKey, readValue, readActive)) {
            if (readActive && readKey == key && readValue == value) {
                // Mark as deleted
                file.seekp(pos);
                writeEntry(file, key, value, false);
                file.flush();
                return;
            }
            pos = file.tellg();
        }
    }
    
    vector<int> find(const string& key) {
        int bucket = get_bucket(key);
        ensureOpen(bucket);
        fstream& file = bucketFiles[bucket];
        
        vector<int> results;
        file.seekg(0, ios::beg);
        string readKey;
        int readValue;
        bool readActive;
        
        while (readEntry(file, readKey, readValue, readActive)) {
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
