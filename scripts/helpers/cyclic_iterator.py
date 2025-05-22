def cyclic_iterator(lst, start_pos=0):
    if not lst:
        raise RuntimeError("Not a list")

    if start_pos >= len(lst):
        raise RuntimeError("Invalid starting position")

    # First yield elements from start_pos to the end
    for i in range(start_pos, len(lst)):
        yield lst[i]

    # Then yield elements from the beginning up to (but not including) start_pos
    for i in range(0, start_pos):
        yield lst[i]
