#ifndef LST_TIMER_H
#define LST_TIMER_H

#include <timer.h>

/**
 * 定时器
 *
 * */

class util_timer;

struct client_data{
    sockaddr_in address;
    int sockfd;
    util_timer* timer;
}

class util_timer{
public:
    util_timer():prev(NULL), next(NULL){}

public:
    time_t expire;
    void  (*cb_func)(client_data *);
    client_data* user_data;
    util_timer* prev;   //定义前继节点
    util_timer* next;   //定义后继节点

};

class sort_timer_list{
public:
    sort_timer_list():head(NULL), tail(NULL){}

    ~sort_timer_list(){
        util_timer* tmp = head;
        while(tmp){
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
        tmp = NULL;
    }
    
    //添加新的节点
    void add_timer(util_timer* timer){
        if(!timer){
            return;
        }
        // 空链表直接初始化，并插入timer
        if(!head){
            head = tail = timer;
            return;
        }
        // 如果这个链表已经有节点，
        if(timer->expire < head->expire){
            timer->next =  head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer, head);
    }

    //调整节点位置
    void adjut_timer(util_timer* timer){
        // 如果timer  节点为空
        if(!timer){
            return;
        }
        //如果节点是尾节点或者位置已经是正确的
        util_timer* tmp = timer->next;
        if(!tmp || (timer->expire < tmp->expire)){
            return;
        }
        //如果timer 节点是head 节点，就要把head往下移动，把这个timer挑出来，然后调用add_timer
        if(timer == head){
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }else{
            timer->prev->next = timer->next;
            timer-next->prev = timer->prev;
            add_timer(timer, head);
        }
    }
    // 删除节点
    void del_timer(util_timer* timer){
        if(!timer){
            return;
        }
        //如果只有一个节点
        if(timer == head && timer == tail){
            delete timer;
            head = NULL;
            tail = NULL;
        }
        //删除头节点
        if(timer == head){
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        //删除尾节点
        if(timer == tail){
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        //删除的节点是中间节点
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            delete timer;
        }
        void tick(){
            if(!head){
                return;
            }
            printf("timer tick\n");
            //获取当前时间
            time_t cur = time(NULL);
            util_timer *tmp = head;
            while(tmp){
                //如果没有超时 break
                if(cur < tmp->expire){
                    break;
                }
                tmp->cb_func(tmp->user_data);
                head = tmp->next;
                if(head){
                    head->prev = NULL;
                }
                delete tmp;
                tmp = head;
            }
        }

private:
        void add_timer(util_timer* timer, util_timer* lst_head){
            util_timer* prev = lst_head;
            util_timer* tmp = prev->next;
            while(tmp){
                if(timer->expire < tmp->expire){
                    prev->next = timer;
                    timer->next = tmp;
                    tmp->prev = timer;
                    timer->prev = prev;
                }
                prev = tmp;
                tmp = tpm->next;
            }
            if(!tmp){
                prev->next = timer;
                timer->prev = prev;
                timer->next = NULL;
                tail = timer;
            }
        }
private:
        util_timer* head;
        util_timer* tail;
};
#endif






