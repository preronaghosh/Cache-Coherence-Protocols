#include <iostream>
#include <unordered_map>
#include <vector>

// Define Cache Line States
enum class State {
    Modified,
    Invalid
};

// Define a structure for a Cache Line
struct CacheLine {
    int data;
    State state;
};

// Bus operations
enum class BusOp {
    GetS,
    GetM,
    PutM
};
// Forward declaration of Cache to resolve circular dependency
class Cache;

// Bus class
class Bus {
private:
    std::vector<Cache*> caches;

public:
    // Register a cache with the bus
    void connect(Cache* cache) {
        caches.push_back(cache);
    }

    // Broadcast a bus operation to all caches
    void broadcast(BusOp operation, int address, int originId) {
        for (auto cache : caches) {
            if (cache->getId() != originId) {
                cache->snoop(operation, address);
            }
        }
    }
};

// Cache class
class Cache {
private:
    std::unordered_map<int, CacheLine> cacheLines; // Cache lines with their states
    Bus* bus;
    int id; // Unique identifier for each cache

public:
    Cache(int id, Bus* bus) : id{id}, bus{bus} {}

    // Function to process a read operation
    void read(int address) {
        if (cacheLines.count(address) && cacheLines[address].state == State::Modified) {
            std::cout << "Cache " << id << ": Read Hit for address " << address << std::endl;
        } else {
            std::cout << "Cache " << id << ": Read Miss for address " << address << std::endl;
            bus->broadcast(BusOp::GetS, address, id);
        }
    }

    // Function to process a write operation
    void write(int address, int value) {
        if (cacheLines.count(address) && cacheLines[address].state == State::Modified) {
            std::cout << "Cache " << id << ": Write Hit for address " << address << std::endl;
            cacheLines[address].data = value;
        } else {
            std::cout << "Cache " << id << ": Write Miss for address " << address << std::endl;
            bus->broadcast(BusOp::GetM, address, id);
            cacheLines[address] = {value, State::Modified};
        }
    }

    // Invalidate a cache line
    void invalidate(int address) {
        if (cacheLines.count(address)) {
            cacheLines[address].state = State::Invalid;
            std::cout << "Cache " << id << ": Invalidated address " << address << std::endl;
        }
    }

    // Write back to memory and invalidate
    void putM(int address) {
        if (cacheLines.count(address) && cacheLines[address].state == State::Modified) {
            std::cout << "Cache " << id << ": Writing back and invalidating address " << address << std::endl;
            // Here you would write back to main memory
            cacheLines[address].state = State::Invalid;
        }
    }

    // Update cache on bus operation
    void snoop(BusOp operation, int address) {
        switch (operation) {
            case BusOp::GetS:
                // Another core wants to share, downgrade if Modified
                if (cacheLines[address].state == State::Modified) {
                    putM(address);
                }
                break;
            case BusOp::GetM:
                // Another core wants to modify, invalidate if present
                invalidate(address);
                break;
            default:
                // Ignore other operations
                break;
        }
    }

    // Method to get the unique identifier for the cache
    int getId() const {
        return id;
    }
};

// Main function to simulate the caches and bus
int main() {
    Bus bus;
    Cache cache1(1, &bus);
    Cache cache2(2, &bus);

    bus.connect(&cache1);
    bus.connect(&cache2);

    cache1.read(0x1A);   // Cache 1 reads from address 0x1A (miss)
    cache1.write(0x1A, 5); // Cache 1 writes to address 0x1A
    cache2.read(0x1A);   // Cache 2 reads from address 0x1A (should invalidate Cache 1)
    cache2.write(0x1B, 10); // Cache 2 writes to a different address 0x1B

    return 0;
}
