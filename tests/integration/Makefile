# Makefile for mod_random integration tests

.PHONY: all test clean start stop logs help

all: test

# Run all integration tests
test:
	@echo "Running integration tests..."
	@python3 run_tests.py

# Start Apache server manually (for debugging)
start:
	@echo "Starting Apache test server on http://localhost:8888"
	@apache2 -f $(shell pwd)/conf/httpd.conf -DFOREGROUND

# Stop Apache server
stop:
	@if [ -f logs/httpd.pid ]; then \
		kill `cat logs/httpd.pid` 2>/dev/null || true; \
		echo "Apache stopped"; \
	else \
		echo "No Apache process found"; \
	fi

# View logs in real-time
logs:
	@echo "=== Error Log ==="
	@tail -f logs/error.log

# View access logs
access:
	@echo "=== Access Log ==="
	@tail -f logs/access.log

# Clean logs and temporary files
clean:
	@rm -rf logs/*
	@echo "Logs cleaned"

# Test specific endpoint with curl
test-endpoint:
	@echo "Testing endpoint: $(ENDPOINT)"
	@curl -v http://localhost:8888/$(ENDPOINT)

# Quick test (start server, run tests, stop)
quick:
	@$(MAKE) stop || true
	@sleep 1
	@$(MAKE) test

# Check Apache configuration
check:
	@apache2 -f $(shell pwd)/conf/httpd.conf -t

# List all test endpoints
list:
	@echo "Available test endpoints:"
	@grep -E "^<Location" conf/httpd.conf | sed 's/<Location "/  - /' | sed 's/">//'

# Help
help:
	@echo "mod_random Integration Tests - Make targets:"
	@echo ""
	@echo "  make test          - Run all integration tests"
	@echo "  make start         - Start Apache server manually"
	@echo "  make stop          - Stop Apache server"
	@echo "  make logs          - View error logs (real-time)"
	@echo "  make access        - View access logs (real-time)"
	@echo "  make clean         - Clean logs"
	@echo "  make check         - Validate Apache configuration"
	@echo "  make list          - List all test endpoints"
	@echo "  make quick         - Quick test (stop, test, cleanup)"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make test"
	@echo "  make start &"
	@echo "  curl http://localhost:8888/test1-basic"
