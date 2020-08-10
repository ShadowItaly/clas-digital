all:
	cd build && make

reset_static: reset_static_books reset_static_catalogue
reset_static_books:
	-sudo rm /etc/clas-digital-devel/updated_static_books.txt
reset_static_catalogue:
	-sudo rm /etc/clas-digital-devel/updated_catalogue.txt


reset_static_stable: reset_static_books_stable reset_static_catalogue_stable
reset_static_books_stable:
	-sudo rm /etc/clas-digital/updated_static_books.txt
reset_static_catalogue_stable:
	-sudo rm /etc/clas-digital/updated_catalogue.txt

log:
	sudo journalctl -f -u clas-digital-devel

log_stable:
	sudo journalctl -f -u clas-digital

.PHONY: log log_stable

install:
	cd build && sudo cmake --build . --target install

install_stable:
	cd build && rm CMakeCache.txt && cmake .. -DSTABLE_INSTALL=TRUE && cmake --build . && sudo cmake --build . --target install && rm CMakeCache.txt 


install_service:
	sudo cp src/clas-digital.service /etc/systemd/system
