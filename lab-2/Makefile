#   make                  собрать (QCustomPlot скачивается при первой сборке)
#   make run              собрать и запустить; порт можно задать: make run PORT=/dev/ttys004
#   make ports            поднять виртуальную пару портов для проверки без платы
#   make fake PORT=...    слать в порт пилообразный сигнал, как от платы
#   make clean            удалить сборку

QMAKE ?= qmake
BAUD  ?= 19200

QCP_VERSION := 2.1.1
QCP_URL     := https://www.qcustomplot.com/release/$(QCP_VERSION)/QCustomPlot-source.tar.gz
QCP_DIR     := monitor/qcustomplot
QCP_FILES   := $(QCP_DIR)/qcustomplot.h $(QCP_DIR)/qcustomplot.cpp

BUILD_DIR := monitor/build

ifeq ($(shell uname),Darwin)
APP := $(BUILD_DIR)/monitor.app/Contents/MacOS/monitor
else
APP := $(BUILD_DIR)/monitor
endif

.PHONY: all deps build run ports fake clean

all: build

deps: $(QCP_FILES)

$(QCP_FILES):
	mkdir -p $(QCP_DIR)
	curl -fsSL $(QCP_URL) | tar xzf - -C $(QCP_DIR) --strip-components=1 \
		qcustomplot-source/qcustomplot.h \
		qcustomplot-source/qcustomplot.cpp \
		qcustomplot-source/GPL.txt

$(BUILD_DIR)/Makefile: monitor/monitor.pro | $(QCP_FILES)
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(QMAKE) ../monitor.pro

build: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

run: build
	$(APP) $(if $(PORT),--port $(PORT) --baud $(BAUD))

# Печатает два пути /dev/ttys00N: один открыть в мониторе, второй передать в make fake.
ports:
	socat -d -d pty,raw,echo=0 pty,raw,echo=0

fake:
	@test -n "$(PORT)" || { echo "make fake PORT=/dev/ttys00N"; exit 1; }
	i=0; while :; do printf '%d\r\n' $$(( i * 41 % 4096 )); i=$$((i + 1)); sleep 0.02; done > $(PORT)

clean:
	rm -rf $(BUILD_DIR)
