package com.redsafetw.edge_service.service;

import com.redsafetw.edge_service.dto.EdgeRenameRequest;
import com.redsafetw.edge_service.repository.EdgeRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

/**
 * 邊緣裝置重新命名服務
 *
 * @create 2025-09-14
 */
@Service
@Transactional
@RequiredArgsConstructor
public class EdgeRenameService {
    private final EdgeRepository edgeRepository;

    public Boolean rename(EdgeRenameRequest edgeRenameRequest) {

//        if (!edgeRepository.existsByEdgeId(edgeRenameRequest.getEdgeId())) {
//            return false;
//        }
//
//        var edgeOptional = edgeRepository.findByEdgeId(edgeRenameRequest.getEdgeId());
//        if (edgeOptional.isEmpty()) {
//            return false;
//        }
//
//        var edge = edgeOptional.get();
//
//        edge.setEdgeName(edgeRenameRequest.getEdgeName());
//        edgeRepository.saveAndFlush(edge);
//        return true;

        // GPT
        return edgeRepository.findByEdgeId(edgeRenameRequest.getEdgeId())
                .map(edge -> {
                    edge.setEdgeName(edgeRenameRequest.getEdgeName());
                    edgeRepository.saveAndFlush(edge);
                    return true;
                })
                .orElse(false);
    }
}