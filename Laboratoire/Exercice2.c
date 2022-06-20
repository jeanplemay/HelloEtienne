void threadFunction(void) {
	while(true) {
		counter++;

		os_thread_yield();
	}
	// You must not return from the thread function
}