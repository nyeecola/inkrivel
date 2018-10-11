compile_only: FORCE
	@make --no-print-directory -C chat server &
	@make --no-print-directory -C game_client &
	@make --no-print-directory -C game_server &
	@make --no-print-directory -C lobby

server_run_no_stop: FORCE
	@make --no-print-directory -C chat run_server &
	@make --no-print-directory -C game_server run &
	@make --no-print-directory -C server run &

client_run: FORCE
	@make --no-print-directory -C game_client & 
	@make --no-print-directory -C lobby run

server_run:
	make server_run && make stop

stop:
	killall game_server & killall chat_server & killall ruby

clean:
	rm -rf chat/bin game_client/bin game_server/bin lobby/bin imgui.ini lobby/imgui.ini

FORCE:
