# cache-coherence-protocols
Bus-based shared memory protocols in multicore systems (MI, MSI and MESI)

### Operations that take place:

- Load Hit
- Load Miss

- Store Hit
- Store Miss

- Events on Bus:
    - GetS: Another core load miss
    - GetM: Another core store miss (local core has to send data and transition to some state)

- Only when private caches don't have that data (all are in I state), main memory will send the data. 
- If there is a shared cache (usually L3), it also maintains similar states and marks that data as dirty or clean wrt to main memory after sending it out. 

