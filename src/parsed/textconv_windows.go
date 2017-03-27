package main

func convertString(text string) string {
	l, _ := ToUTF8("gbk", []byte(text))
	return string(l)
}

func convertByteArray(text []byte) []byte {
	l, _ := ToUTF8("gbk", text)
	return l
}
