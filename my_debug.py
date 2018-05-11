import lldb

def on_replacement_malloc(frame, bp_loc, dict):
    # print("in on_replacement_malloc")
    if frame.thread is None:
        print("frame.thread is None?")
        return False
    # for f in frame.thread:
    #     print(f)
    the_list = [x for x in frame.thread if x.function.name is not None and 'replacement_malloc' in x.function.name]
    # print("len of the list: %d" % len(the_list))
    if len(the_list) > 10:
        print("len of the list is > 10: %d" % len(the_list))
        print("stop in thread %s" % str(frame.thread.GetIndexID()))
        return True
    return False
