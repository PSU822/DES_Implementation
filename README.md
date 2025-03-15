# DES_Implementation
DES implementation built for school assignments, so refactoring has not been carried out.

This program has been confirmed to work in Visual Studio 2022.

---

> 이 프로젝트는 DES 알고리즘을 기재해둔 여러 오픈소스와 미리 구현된 코드를 참고하여 만들어졌습니다.
> 대학교 과제용 프로젝트입니다. 

> **주의**
> 본 프로그램은 실행시에 암호화 했던 파일만 복호화가 가능합니다.
> 암호화 시에 .txt 파일과 .bin 파일을 생성하며, 복호화 시에는 .bin 파일을 입력해야 정상적으로 복호화가 진행됩니다.

# 설계 의도

 AES가 등장하기 전, 잘 짜여진 보안의 표준인 DES를 구현했습니다.
각각 알고리즘 자체들을 단계 별로 세분화하여 함수화 하고, 헤더파일에 명시하여 작은 라이브러리처럼 쓰이게 만들었습니다.

  DES를 연산 할 때 들어가는 대표적인 고정된 값들이 있습니다. shift 연산과 s-box의 연산은 이러한 정보를 요구하기에 따로 프로그램이 참고하여 사용 할 수 있게,
 테이블로 선언하여 정리해 두었습니다. 이는 ```DES_value.h``` 에 선언되어있습니다.

  라이브러리의 형색을 갖추게 설계한 만큼, 함수 각각들의 오류를 바로 잡고 의도하지 않은 과정을 멈출 수 있게 하기 위해 예외 / 오류 처리의 기능도 존재합니다.
이를 위한 함수들은 ```DES_interface.h``` 에 선언되어있습니다.

 사용자가 사용할 프로그램은 존재해야 하므로, 구현한 라이브러리로 만든 사용자 인터페이스와 그 인터페이스 내부에서 쓸 함수들은 ```DES_for_user.h``` 에 선언되어있습니다.
즉, 사전으로 정의된 프로그램을 이용하려면 ```Main.c``` 를 작동시키면 됩니다.

 여러 함수 테스트, 설계 의도는 대부분 주석처리 되어있으나, ```TestCode.c``` 를 참조하면 각 함수별 처리 이유가 주석처리 되어있습니다.

 ---

# 작동 방식

 ```Main.c``` 는 간단한 인터페이스를 보여주므로, 커맨드 라인 애플리케이션으로 동작하여 직관적으로 사용 하실 수 있습니다.
파일 이름을 입력하여 암호화 / 복호화 하는 것은 전치암호 프로젝트와 동일하지만, 굳이 개행을 나누는 방식을 채택하지 않았습니다.
위에서도 언급한 **주의사항으로는, 복호화 시킬 파일은 .bin 확장자를 꼭 명시해주셔야 합니다.**

**```DES_interface.h``` 와 ```DES_values.h```는 같이 사용 해 주시길 바랍니다.
인터페이스는 언급했듯 함수가 구현된 라이브러리 격 헤더이며, values는 각 함수가 참고해야 할 정보들이 담긴 테이블 헤더입니다.**

---

# 개선점 및 아쉬운 점

  전치암호 이후 프로젝트 인 만큼, 구조체의 취약점 개선 같은 부분과 테스트 코드를 작성하며 미리 오류를 캐치하여 개선하고, 잘 구현된 오픈소스들을 공부하며
 조금 더 체계적인 구현이 되었다는 점은 좋지만.

- 예외 상황에 대한 처리가 다 똑같이 되어버렸다.
- .txt 파일을 받아 .bin을 복호화 하는 건 사용자의 관점에서 매우 불편할 수 있다.
- 암호화/복호화의 공통되는 부분을 함수화 할 수 있었을지도 모른다는 생각이 또 발생했다.
- 동적 메모리 그 자체로 생기는 실수 시 누수될 가능성이 있다.

 리팩토링을 진행한다면 제일 먼저 파일 확장자를 자동으로 처리하게 하지 않을 까 싶다. 
암호화/복호화 상태를 받는 함수와, 파일 이름을 받아서 자동으로 파일 이름을 암/복호화 함수에 넘겨줄 때 .txt, .bin을 붙이게끔 만든다면 훨씬 가용성이 좋지 않을 까 싶다.
혹은 bin 파일을 쓰지 않고도 완벽하게 동작하게 만들고 싶지만, 의도한 eof를 체크하여 파일을 읽거나 띄워쓰기를 캐치하지 못 하고 null로 선언해버리는 문제는 여전했다.

 두 번째로는 예외사항의 메세지를 더 다양하게 출력하려고 한다. 이는 디버깅할때 큰 도움이 되며, 라이브러리의 구조를 띄게 설계한 만큼 중요했는데 놓친 부분이라 생각된다.
