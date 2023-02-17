define dump_heap_internal
	set $heap = ($arg0)->heap
	set $entry = (struct AllocEntry*)$heap
	while 1
		printf "Size = %d Free %d", $entry->size, $entry->free
		printf "\n"

		if $entry->next == 0
			loop_break
		end

		set $position = (char*)$entry
		set $position = $position + sizeof(struct AllocEntry) + $entry->size
		set $entry = (struct AllocEntry*)$position
	end
end

define dump_heap
	dont-repeat
	if $argc != 1
		echo "Give me heap address"
	else
		dump_heap_internal $arg0
	end
end

