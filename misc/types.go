package misc

import "encoding/json"

func StructToJsonTagMap(data interface{}) (map[string]interface{}, error) {
	result := make(map[string]interface{})

	b, err := json.Marshal(data)
	if err != nil {
		return nil, err
	}

	err = json.Unmarshal(b, &result)
	if err != nil {
		return nil, err
	}

	return result, nil
}
