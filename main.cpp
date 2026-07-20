#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <functional>

using namespace std;

const int NUM_BUCKETS = 19;
const string DATA_FILE_PREFIX = "bucket_";

// Simple hash function
size_t hash_string(const string& str) {
    size_t h = 0;
    for (char c : str) {
        h = h * 31 + c;
    }
    return h;
}

int get_bucket(const string& key) {
    return hash_string(key) % NUM_BUCKETS;
}

string get_bucket_filename(int bucket) {
    return DATA_FILE_PREFIX + to_string(bucket) + ".dat";
}

struct FileEntry {
    char key[65];
    int value;
    bool active;
    int next; // Position of next entry in same bucket chain, -1 if end
};

class FileStorage {
private:
    vector<fstream> bucketFiles;
    const int ENTRY_SIZE = sizeof(FileEntry);
    
    void writeEntry(fstream& file, int pos, const FileEntry& entry) {
        file.seekp(pos);
        file.write(reinterpret_cast<const char*>(&entry), ENTRY_SIZE);
    }
    
    bool readEntry(fstream& file, int pos, FileEntry& entry) {
        file.seekg(pos);
        return !!file.read(reinterpret_cast<char*>(&entry), ENTRY_SIZE);
    }
    
    int appendEntry(fstream& file, const FileEntry& entry) {
        file.seekp(0, ios::end);
        int pos = file.tellp();
        file.write(reinterpret_cast<const char*>(&entry), ENTRY_SIZE);
        return pos;
    }
    
public:
    FileStorage() {
        bucketFiles.resize(NUM_BUCKETS);
        for (int i = 0; i < NUM_BUCKETS; i++) {
            bucketFiles[i].open(get_bucket_filename(i), 
                               ios::in | ios::out | ios::binary);
            if (!bucketFiles[i]) {
                bucketFiles[i].open(get_bucket_filename(i), 
                                   ios::out | ios::binary);
                bucketFiles[i].close();
                bucketFiles[i].open(get_bucket_filename(i), 
                                   ios::in | ios::out | ios::binary);
            }
        }
    }
    
    ~FileStorage() {
        for (auto& file : bucketFiles) {
            if (file.is_open()) {
                file.close();
            }
        }
    }
    
    void insert(const string& key, int value) {
        int bucket = get_bucket(key);
        fstream& file = bucketFiles[bucket];
        
        // Check if entry already exists
        FileEntry entry;
        int pos = 0;
        while (readEntry(file, pos, entry)) {
            if (entry.active && strcmp(entry.key, key.c_str()) == 0 && entry.value == value) {
                return; // Already exists
            }
            pos += ENTRY_SIZE;
        }
        
        // Create new entry
        FileEntry newEntry;
        strncpy(newEntry.key, key.c_str(), 64);
        newEntry.key[64] = '\0';
        newEntry.value = value;
        newEntry.active = true;
        newEntry.next = -1;
        
        appendEntry(file, newEntry);
        file.flush();
    }
    
    void remove(const string& key, int value) {
        int bucket = get_bucket(key);
        fstream& file = bucketFiles[bucket];
        
        FileEntry entry;
        int pos = 0;
        while (readEntry(file, pos, entry)) {
            if (entry.active && strcmp(entry.key, key.c_str()) == 0 && entry.value == value) {
                // Mark as deleted
                entry.active = false;
                writeEntry(file, pos, entry);
                file.flush();
                return;
            }
            pos += ENTRY_SIZE;
        }
    }
    
    vector<int> find(const string& key) {
        int bucket = get_bucket(key);
        fstream& file = bucketFiles[bucket];
        
        vector<int> results;
        FileEntry entry;
        int pos = 0;
        
        while (readEntry(file, pos, entry)) {
            if (entry.active && strcmp(entry.key, key.c_str()) == 0) {
                results.push_back(entry.value);
            }
            pos += ENTRY_SIZE;
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
