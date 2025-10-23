package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.UserDomain;
import com.redsafetw.user_service.dto.GetUserIdResponse;
import com.redsafetw.user_service.repository.UserRepository;
import lombok.RequiredArgsConstructor;
import org.apache.catalina.User;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.util.Optional;

@Service
@RequiredArgsConstructor
public class GetUserIdService {
    private final UserRepository userRepository;

    public GetUserIdResponse getUserIdByEmail(String email) {
        Optional<UserDomain> user = userRepository.findByEmail(email);

        if (user.isPresent()) {
            return GetUserIdResponse.builder().userId(user.get().getUserId()).build();
        }

        throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "EMAILNOTFOUND");
    }
}
