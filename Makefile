exe = faketty
exe-src = faketty.c
man = faketty.1.gz
man-src = faketty.1.in
version = 0.0.1

.PHONY: all clean

all: $(exe) $(man)

$(exe): $(exe-src)
	gcc	-std=gnu99 -Wall -Wextra \
		-pedantic \
		-shared -fPIC -pie \
		-O3 \
		-DVERSION='"$(version)"' \
		-o $@ $< \
		-ldl

$(man): $(man-src)
	sed 's/@@VERSION@@/$(version)/g' $< | gzip > $@

clean:
	rm -f $(exe) $(man)
