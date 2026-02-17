import json

with open('long.json', 'r') as f:
    json_data = json.load(f)

data = ""
hid_data_lenths = []
for i, packet in enumerate(json_data):
    hid_data = packet["_source"]["layers"]["usbhid.data"]
    hid_data = hid_data.replace(":", "")
    if hid_data != data:
        data = hid_data
    else:
        continue
    if not hid_data.startswith("600042"):
        continue
    hid_data_lenths.append(int(len(hid_data)/2))
    
    # Convert hex string to C byte array
    bytes_list = [f"0x{hid_data[j:j+2]}" for j in range(0, len(hid_data), 2)]
    bytes_str = ", ".join(bytes_list)
    
    print(hid_data)
    #print(f'uint8_t rpt{i}[] = {{{bytes_str}}};')
    #print(f"sendIntOut64(rpt{i});")
    #print("delay(1000);")
    #print(f"Serial.println({i});")

print(f"// Lengths: {hid_data_lenths}")