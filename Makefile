bin: FORCE
	@make --no-print-directory -C chat server &
	@make --no-print-directory -C game_client & 
	@make --no-print-directory -C game_server &
	@make --no-print-directory -C lobby

bin_run: FORCE
	@make --no-print-directory -C chat run_server &
	@make --no-print-directory -C game_client & 
	@make --no-print-directory -C game_server run &
	@make --no-print-directory -C lobby run

run:
	make bin_run && make stop

stop:
	killall game_server & killall chat_server

clean:
	rm -rf chat/bin game_client/bin game_server/bin lobby/bin imgui.ini lobby/imgui.ini

FORCE:
