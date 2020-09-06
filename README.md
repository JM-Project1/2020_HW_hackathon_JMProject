## 팀명 및 팀원

### 팀명

JMProject

### 팀원

* 팀원이 맡은 역할을 자유롭게 적어주세요(기획/디자인/설계/개발/디버깅/기술조언/환경설정/발표 등)
* 김명규, @MyeongQkim - H/W Developer
* 석종일, @daclouds - S/W Developer

## 프로젝트 제목 

* 코로나 시대, 택배 도착 알림 서비스

## 프로젝트 배경 혹은 목적 

```
코로나로 인해 온라인 쇼핑이 늘었고 이로 인해 수많은 곳에서 택배를 수령하게 되었습니다.
이때 택배 기사들의 움직임을 감지해 택배가 잘 도착했는지 확인해주는 시스템을 만들게 되었습니다.
```

## 파일 리스트 

* dashboard/res/server.js
* camera/inc/controller_send.h
* camera/src/controller_send.c
* api/src/app.js

## 코드 기여자 

* api/src/app.js, 종일
* dashboard/res/server.js, #L68-L72 latest 추가, 종일
* camera/inc/controller_send.h, 종일
* camera/src/controller_send.c, 종일

## 보드 

* RPI4 : 센서 연동, 카메라, iotjs

## 구현사항(가산점) 

* Peripheral GPIO

## 참고

- [워크샵 주요 일정](workshop.md)

### 구성도

```
+-------------------------------------------------+
| +-----------------+ sensor +------------------+ |           +-----+    +------+
| | Peripheral GPIO |------->| dashboard(iotjs) | |---------->| api |--->| push |
| +-----------------+        +------------------+ |  (image)  +-----+    +------+
|                      RPI4                       |     |        |
+-------------------------------------------------+     |        | login
                                           +--------+   |   +---------+
                                           | camera |---+   | browser |
                                           +--------+       +---------+
```

### 동작 설명

* 브라우저를 통해 API 서버에서 Kakao 계정에 Login 한다. (이 사용자에게 PUSH 를 보낸다.)
* Motion sensor 를 통해 사람/물체를 감지하고 
```
* 처음 아무 것도 없던 상태 (Sensor 동작 X)
* 택배 기사가 물건을 가져 다 놓음 (모션 인식 O)
* 택배 기사가 물건을 두고 감 (Sensor 동작 X)
```
* 마지막 Sensor 가 동작을 안하는 시점에 카메라를 켜고 촬영을 한 다음 `curl` API 서버에 요청을 보내고 카메라를 끕니다.
* API 서버는 dashboard 에 요청을 보내 마지막으로 저장된 이미지를 받아 오고 KAKAO API 를 통해 PUSH 알림을 보냅니다.
* 마지막 촬영된 이미지를 Kakao API 를 통해 PUSH 알림으로 발송 합니다.