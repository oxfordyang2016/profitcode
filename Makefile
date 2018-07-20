WAF = backend/tools/waf

init_test:
	$(WAF) configure init_test $(PARAMS)

tools:
	$(WAF) configure tools $(PARAMS)

simtrade:
	$(WAF) configure simtrade $(PARAMS)

clean:
	rm -rf build
